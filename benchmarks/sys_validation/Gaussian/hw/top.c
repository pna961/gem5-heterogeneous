#include "hw_defines.h"

void top(uint64_t GAUS_addr) {

	//Define Device MMRs
	volatile uint8_t  * GAUSFlags  = (uint8_t *)GAUSSIAN;
	volatile uint8_t  * DmaFlags   = (uint8_t  *)(DMA_Flags);
	volatile uint64_t * DmaRdAddr  = (uint64_t *)(DMA_RdAddr);
	volatile uint64_t * DmaWrAddr  = (uint64_t *)(DMA_WrAddr);
	volatile uint32_t * DmaCopyLen = (uint32_t *)(DMA_CopyLen);

	//Transfer Input Matrices
	//Transfer Gaussian_matrix
	*DmaRdAddr  = GAUS_addr;
	*DmaWrAddr  = Gaussian_matrix;
	*DmaCopyLen = N * N * sizeof(double);
	*DmaFlags   = DEV_INIT;
	//Poll DMA for finish
	while ((*DmaFlags & DEV_INTR) != DEV_INTR);

	//Start the accelerated function
	*GAUSFlags = DEV_INIT;
	//Poll function for finish
	while ((*GAUSFlags & DEV_INTR) != DEV_INTR);

	//Transfer Gaussian_matrix back
	*DmaRdAddr  = Gaussian_matrix;
	*DmaWrAddr  = GAUS_addr;
	*DmaCopyLen = N * N * sizeof(double);
	*DmaFlags   = DEV_INIT;
	//Poll DMA for finish
	while ((*DmaFlags & DEV_INTR) != DEV_INTR);
	
	return;
}