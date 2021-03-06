/* FreeEMS - the open source engine management system
 *
 * Copyright 2009 Sean Keys
 *
 * This file is part of the FreeEMS project.
 *
 * FreeEMS software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeEMS software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with any FreeEMS software.  If not, see http://www.gnu.org/licenses/
 *
 * We ask that if you make any changes to this file you email them upstream to
 * us at admin(at)diyefi(dot)org or, even better, fork the code on github.com!
 *
 * Thank you for choosing FreeEMS to run your engine!
 */


/**	@file LT1-360-8.c
 * @ingroup interruptHandlers
 * @ingroup enginePositionRPMDecoders
 *
 * @brief LT1 Optispark
 *
 * Uses PT1 to interrupt on rising and falling events of the 8x cam sensor track.
 * A certain number of 360x teeth will pass while PT1 is in a high or low state.
 * Using that uniquek count we can set the positing of your Virtual CAS clock.
 * After VCAS's position is set set PT7 to only interrupt on every 5th tooth, lowering
 * the amount of interrupts generated, to a reasonable level.
 *
 * @note Pseudo code that does not compile with zero warnings and errors MUST be commented out.
 *
 * @todo TODO This file contains SFA but Sean Keys is going to fill it up with
 * @todo TODO wonderful goodness very soon ;-)
 *
 * @author Sean Keys
 */


#define LT1_360_8_C
#include "inc/freeEMS.h"
#include "inc/interrupts.h"
#include "inc/LT1-360-8.h"


unsigned short VCAS = 0;  /* create our virtual Cam Angle Sensor */

/** Setup PT Capturing so that we can decode the LT1 pattern
 *  @todo TODO Put this in the correct place
 *
 */
void LT1PTInit(void){
	/* set pt1 to capture on rising and falling */

}


/*	set overflow to 8 so that it will fire an interrupt on every 8th tooth, basically making it a 45tooth CAS */
/** Primary RPM ISR
 *
 * @todo TODO Docs here!
 */
void PrimaryRPMISR(void){
	/* Clear the interrupt flag for this input compare channel */
	TFLG = 0x01;

	/* Save all relevant available data here */
//	unsigned short codeStartTimeStamp = TCNT;		/* Save the current timer count */
//	unsigned short edgeTimeStamp = TC0;				/* Save the edge time stamp */
	unsigned char PTITCurrentState = PTIT;			/* Save the values on port T regardless of the state of DDRT */
//	unsigned short PORTS_BACurrentState = PORTS_BA;	/* Save ignition output state */

//	unsigned char risingEdge; /* in LT1s case risingEdge means signal is high */
//	if(fixedConfigs1.coreSettingsA & PRIMARY_POLARITY){
//		risingEdge = PTITCurrentState & 0x01;
//	}else{
//		risingEdge = !(PTITCurrentState & 0x01);
//	}

	PORTJ |= 0x80; /* Echo input condition on J7 */
	if(!isSynced){  /* If the CAS is not in sync get window counts so SecondaryRPMISR can set position */
		if (PTITCurrentState & 0x02){
			PrimaryTeethDuringHigh++;  /* if low resolution signal is high count number of pulses */
		}else{
			PrimaryTeethDuringLow++;  /* if low resolution signal is low count number of pulses */
		}
	}else{  /* The CAS is synced and we need to update our 360/5=72 tooth wheel */
		/// @todo TODO possibly add code to make sure we are in divide mode, if not error out
		VCAS = VCAS + 10;  /* Check/correct for 10deg of CAM movement */
		/// @todo TODO fill in or remove the else
	}
}


/** Secondary RPM ISR
 * @brief Use the rising and falling edges...................
 * @todo TODO Docs here!
 * @todo TODO Add a check for 1 skip pulse of the 8x track, to prevent possible incorrect sync.
 * @todo TODO Possibly make virtual CAS 16-bit so was can get rid of floating points and use for syncing
 */
void SecondaryRPMISR(void){
	/* Clear the interrupt flag for this input compare channel */
	TFLG = 0x02;

	/* Save all relevant available data here */
//	unsigned short codeStartTimeStamp = TCNT;		/* Save the current timer count */
//	unsigned short edgeTimeStamp = TC1;				/* Save the timestamp */
	unsigned char PTITCurrentState = PTIT;			/* Save the values on port T regardless of the state of DDRT */
//	unsigned short PORTS_BACurrentState = PORTS_BA;	/* Save ignition output state */
	unsigned char risingEdge;
	if(fixedConfigs1.coreSettingsA & PRIMARY_POLARITY){
			risingEdge = PTITCurrentState & 0x01;
		}else{
			risingEdge = !(PTITCurrentState & 0x01);
		}
	PORTJ |= 0x40;  /* echo input condition */

	if (!isSynced & risingEdge){ /* If the CAS is not in sync get window counts and set virtual CAS position */
		  /* if signal is high that means we can count the lows */
			switch (PrimaryTeethDuringLow){
			case 23: /* wheel is at 0 deg TDC #1, set our virtual CAS to tooth 0 of 720 */
			{
				VCAS = 0 ;
				changeSyncStatus((unsigned char) 1);
				break;
			}
			case 38: /* wheel is at 90 deg TDC #4, set our virtual CAS to tooth 180 of 720 */
			{
				VCAS = 180;
				changeSyncStatus((unsigned char) 1);
				break;
			}
			case 33: /* wheel is at 180 deg TDC #6 set our virtual CAS to tooth 360 of 720 */
			{
				VCAS = 360;
				changeSyncStatus((unsigned char) 1);
				break;
			}
			case 28: /* wheel is at 270 deg TDC #7 set our virtual CAS to tooth 540 of 720 */
			{
				VCAS = 540;
				changeSyncStatus((unsigned char) 1);
				break;
			}
			default :
			{
			Counters.crankSyncLosses++; /* use crankSyncLosses variable to store number of invalid count cases while attempting to sync*/
				break;
			}
			PrimaryTeethDuringLow = 0; /* In any case reset counter */
			}
		}
	if(!isSynced & !risingEdge){/* if the signal is low that means we can count the highs */
			switch (PrimaryTeethDuringHigh){   /* will need to additional code to off-set the initialization of PACNT since they are not
												 evenly divisible by 5 */
			case 7: /* wheel is at 52 deg, 7 deg ATDC #8 set our virtual CAS to tooth 104 of 720 */
			{

				break;
			}
			case 12: /* wheel is at 147 deg, 12 deg ATDC #3 set our virtual CAS to tooth 294 of 720 */
			{

				break;
			}
			case 17: /* wheel is at 242 deg, 17 deg ATDC #5 set our virtual CAS to tooth 484 of 720 */
			{

				break;
			}
			case 22: /* wheel is at 337 deg, 22 deg ATDC #2 set our virtual CAS to tooth 674 of 720 */
			{

			 	break;
			}
			default :
			{
				Counters.crankSyncLosses++; /* use crankSyncLosses variable to store number of invalid/default count cases while attempting to sync*/
				break;
			}

	    	}
			PrimaryTeethDuringHigh = 0;  /* In any case reset counter */
    	}
	if(isSynced & risingEdge){  /* We are in sync and need to make sure our counts are good */

		/* System is synced so use adjusted count numbers to check sync */
		/// @todo TODO fill in or remove the else
	}
}
		//	Counter.primaryTeethAfterSecondaryRise = 0;
		//	Counter.primaryTeethAfterSecondaryFall = 0;

		//	unsigned char risingEdge;
		//		if(fixedConfigs1.coreSettingsA & SECONDARY_POLARITY){
		//			risingEdge = PTITCurrentState & 0x02;
		//		}else{
		//			risingEdge = !(PTITCurrentState & 0x02);
		//		}

/** PT0 Accumulator Mode
 * @brief Change the accumulator mode to overflow every 5 inputs on PT0 making our 360
 *  tooth wheel interrupt like a 72 tooth wheel
 *
 @todo TODO Decide if an explicit parameter is necessary if not use a existing status var instead for now it's explicit.
 */
void changeSyncStatus(unsigned char synced){
	if (synced != 1) { /* disable accumulator counter, so an ISR is fired on all 360 teeth */
		PACTL = 0x00; /* disable PAEN and PBOIV */
		/*	(PACTL) 7   6    5     4     3    2    1    0
			 		  PAEN PAMOD PEDGE CLK1 CLK0 PAOVI PAI */
	}else{  /* enable accumulator so an ISR is only fired on every "5th tooth of the 360x track" */
//		TIOS = TIOS &  "0xCC0x83" WTF!?LOL!! 0x80;  /* PT7 input */
//		TCTL1 = TCTL1 &  "0xCC0x83" WTF!?LOL!! 0xC0; /* Disconnect IC/OC logic from PT7 */
		TIOS = TIOS & 0x80;  /* PT7 input */
		TCTL1 = TCTL1 & 0xC0; /* Disconnect IC/OC logic from PT7 */
		/// @todo TODO the register below does not exist and I couldn't figure out what you meant to do...
		PACN0 = 0xFB ; /* Calculation, $00 – $05 = $FB. This will overflow in 5 more edges. */
		PACTL = 0x52; /* Enable PA in count mode, rising edge and interrupt on overflow  01010010 */
	}
}
