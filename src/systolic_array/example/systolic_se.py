
from __future__ import print_function

import configparser
import argparse
import sys
import os

import m5
from m5.defines import buildEnv
from m5.objects import *
from m5.util import addToPath, fatal

addToPath('./')
addToPath('../')
addToPath('../../../configs')

from ruby import Ruby
from common import Options
from common import Simulation
from common import CacheConfig
from common import CpuConfig
from common import MemConfig
from common.FileSystemConfig import config_filesystem

from common.Caches import *
from common.cpu2000 import *

def addAladdinOptions(parser):
    parser.add_argument("--accel_cfg_file", default=None,
        help="Aladdin accelerator configuration file.")
    parser.add_argument("--aladdin-debugger", action="store_true",
        help="Run the Aladdin debugger on accelerator initialization.")

def createSystolicArrayDatapath(config, accel):
    acceleratorId = config.getint(accel, "accelerator_id")
    peArrayRows = config.getint(accel, "pe_array_rows")
    peArrayCols = config.getint(accel, "pe_array_cols")
    dataType = config.get(accel, "data_type")
    sramSize = config.getint(accel, "sram_size")
    lineSize = config.getint(accel, "line_size")
    fetchQueueCapacity = config.getint(accel, "fetch_queue_capacity")
    commitQueueCapacity = config.getint(accel, "commit_queue_capacity")
    numSpadBanks = config.getint(accel, "num_spad_banks")
    numSpadPorts = config.getint(accel, "num_spad_ports")
    partType = config.get(accel, "partition_type")
    # Set the globally required parameters.
    datapath = SystolicArray(
        acceleratorName = accel,
        acceleratorId = acceleratorId,
        peArrayRows = peArrayRows,
        peArrayCols = peArrayCols,
        dataType = dataType,
        lineSize = lineSize,
        fetchQueueCapacity = fetchQueueCapacity,
        commitQueueCapacity = commitQueueCapacity,
        inputSpad = Scratchpad(
            size = sramSize,
            lineSize = lineSize,
            numBanks = numSpadBanks,
            numPorts = numSpadPorts,
            partType = partType),
        weightSpad = Scratchpad(
            size = sramSize,
            lineSize = lineSize,
            numBanks = numSpadBanks,
            numPorts = numSpadPorts,
            partType = partType),
        outputSpad = Scratchpad(
            size = sramSize,
            lineSize = lineSize,
            numBanks = numSpadBanks,
            numPorts = numSpadPorts,
            partType = partType))
    # Attach the scratchpads and the fetch/commit units of the accelerator
    # to the bus.
    # Input scratchpad.
    for i in range(peArrayRows):
        datapath.input_spad_port[i] = datapath.inputSpadBus.cpu_side_ports
    datapath.inputSpadBus.mem_side_ports = datapath.inputSpad.accelSidePort
    # Weight scratchpad.
    for i in range(peArrayCols):
        datapath.weight_spad_port[i] = datapath.weightSpadBus.cpu_side_ports
    datapath.weightSpadBus.mem_side_ports = datapath.weightSpad.accelSidePort
    # Output scratchpad.
    for i in range(peArrayRows):
        datapath.output_spad_port[i] = datapath.outputSpadBus.cpu_side_ports
    datapath.outputSpadBus.mem_side_ports = datapath.outputSpad.accelSidePort

    setattr(system, datapath.acceleratorName, datapath)

parser = argparse.ArgumentParser()
Options.addCommonOptions(parser)
Options.addSEOptions(parser)
addAladdinOptions(parser)

options = parser.parse_args()

print(options)

if options is None:
    print("Error: script doesn't take any positional arguments")
    sys.exit(1)
    

np = options.num_cpus

(CPUClass, test_mem_mode, FutureClass) = Simulation.setCPUClass(options)

system = System()

# Create a top-level voltage domain
system.voltage_domain = VoltageDomain(voltage = options.sys_voltage)

# Create a source clock for the system and set the clock period
system.clk_domain = SrcClockDomain(clock = options.sys_clock,
                                   voltage_domain = system.voltage_domain)

# Create a CPU voltage domain
system.cpu_voltage_domain = VoltageDomain()

# Create a separate clock domain for the CPUs
system.cpu_clk_domain = SrcClockDomain(clock = options.cpu_clock,
                                       voltage_domain =
                                       system.cpu_voltage_domain)


system.mem_mode = test_mem_mode
system.mem_ranges = [AddrRange(options.mem_size)]
system.cpu = O3CPU()


if options.accel_cfg_file:
    # First read all default values.
    default_cfg = configparser.SafeConfigParser()
    # default_cfg_file = os.path.join(
    #     os.path.dirname(os.path.realpath(__file__)), "aladdin_template.cfg")
    default_cfg_file = "aladdin_template.cfg"
    default_cfg.read(default_cfg_file)
    defaults = dict(i for i in default_cfg.items("DEFAULT"))

    # Now read the actual supplied config file using the defaults.
    config = configparser.SafeConfigParser(defaults)
    config.read(options.accel_cfg_file)
    accels = config.sections()
    if not accels:
        fatal("No accelerators were specified!")
    for accel in accels:
        acceleratorType = config.get(accel, "accelerator_type")
        if (acceleratorType == "systolic_array"):
            createSystolicArrayDatapath(config, accel)
        else:
            fatal("Unknown accelerator type %s!" % acceleratorType)


system.membus = SystemXBar(width=options.xbar_width)
system.system_port = system.membus.cpu_side_ports
CacheConfig.config_cache(options, system)
MemConfig.config_mem(options, system)
config_filesystem(system, options)

if options.cmd:
    system.workload = SEWorkload.init_compatible(options.cmd)
    process = Process()
    process.cmd = [options.cmd]
else:
    print("No workload specified. Exiting!\n")
    #   print >> sys.stderr, "No workload specified. Exiting!\n"
    sys.exit(1)

system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system = False, system = system)
Simulation.run(options, root, system, FutureClass)