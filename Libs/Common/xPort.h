/*
 * xPort.h
 *
 * Created: 16.05.2019 14:29:38
 *  Author: rekuts
 */ 
//==============================================================================
#ifndef XPORT_H_
#define XPORT_H_
//==============================================================================
#include <stdint.h>
#include <stdbool.h>
//==============================================================================
typedef union{
  struct{
    /* Pin0 */  uint32_t Pin0: 1;
    /* Pin1 */  uint32_t Pin1: 1;
    /* Pin2 */  uint32_t Pin2: 1;
    /* Pin3 */  uint32_t Pin3: 1;
    
    /* Pin4 */  uint32_t Pin4: 1;
    /* Pin5 */  uint32_t Pin5: 1;
    /* Pin6 */  uint32_t Pin6: 1;
    /* Pin7 */  uint32_t Pin7: 1;
    
    /* Pin8 */  uint32_t Pin8: 1;
    /* Pin9 */  uint32_t Pin9: 1;
    /* Pin10 */ uint32_t Pin10: 1;
    /* Pin11 */ uint32_t Pin11: 1;
    
    /* Pin12 */ uint32_t Pin12: 1;
    /* Pin13 */ uint32_t Pin13: 1;
    /* Pin14 */ uint32_t Pin14: 1;
    /* Pin15 */ uint32_t Pin15: 1;
  };
  uint32_t Value;
}xPortT;
//==============================================================================
#endif /* XPORT_H_ */
