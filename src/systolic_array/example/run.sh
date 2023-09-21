#!/usr/bin/env bash

bmk_home=./
gem5_dir=../../../

${gem5_dir}build/RISCV/gem5.opt \
  --debug-flags=SystolicToplevel,SystolicDataflow,SystolicFetch,SystolicCommit,SystolicPE,SystolicSpad \
  --outdir=${bmk_home}outputs \
  ${bmk_home}aladdin_se.py \
  --num-cpus=1 \
  --mem-size=4GB \
  --mem-type=SimpleMemory  \
  --sys-clock=1GHz \
  --cpu-type=TimingSimpleCPU \
  --caches \
  --l2cache \
  --l2_size=2MB \
  --l1d_size=65536 \
  --l1i_size=65536 \
  --l2_assoc=16 \
  --l2_hit_latency=20 \
  --cacheline_size=64 \
  --accel_cfg_file=systolic_array.cfg \
  --cmd=test_gem5_accel

