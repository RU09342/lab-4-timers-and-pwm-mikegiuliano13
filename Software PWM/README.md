# Lab 4: Software PWM

## General Structure

This program begins by configuring the two LEDs. The button is configured to enable
interrupts and the frequency calculation function is called to set the debounce wait
period (10 ms) and store it in its associated capture compare register. In this way,
one timer is used to manage debounce timing while another is used to modulate the 
duty cycle.

When the button is pressed, this signals the timer to begin counting immediately and
also clearing the interrupt flag while preventing future ones from occuring.

Within the timer interrupt, the duty cycle is increased and the green LED is toggled on
(pressed down) then off (released). The button interrupt edge is toggled to ensure
proper debounce and at the end of the routine, interrupts are enabled again and the
timer is reset in anticipation of another button press.

All the while, the A1 timer capture compare value is being compared to the current
duty cycle. The red LED is toggled to produce an effective brightness that is
proportional to the rate of toggling.

## Important Distinctions

Generally, differences were in the pinouts for pull-up resistors and buttons.
The MSP430FR2311 and MSP430FR5994 required the use of Timer B instead of Timer A.

LOW POWER MODE MUST BE DISABLED.

### MSP430G2553
//---------------------------------------------------------------------------------------

// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

volatile int state = 0;
volatile int dutycycle = 50; // duty cycle initialized at 50%

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

	// Button configuration
    P1DIR = BIT0 + BIT6; // Set P1.0 and BIT6 as output
    P1REN |= BIT3; // Connects the on-board resistor to P1.3
    P1OUT = BIT3; // Sets up P1.3 as pull-up resistor
    
	// Interrupt Configuration
    P1IES |= BIT3; // Interrupts on button release LO TO HI
    P1IE |= BIT3; // Enable interrupt on button pin
    P1IFG &= ~BIT3; // Clear interrupt flag

	// Timer frequency of 100 Hz --> 10 ms intervals
    frequencyCalc(100);    // initialize timer to 100Hz

    __enable_interrupt(); // MUST BE ENABLED IN ADDITION TO GIE
    __bis_SR_register(GIE); // enable global interrupts
    
	// Compare the current value of the timer in the A1 register
	// with the duty cycle to determine the rate of LED flicker
    while (1) {
        if(TA1R <= dutycycle) 
            P1OUT ^= BIT0;
        else if (TA1R > dutycycle) 
            P1OUT &= ~BIT0;
    }

}

// Sets up the timer compare value to 
void frequencyCalc(int t)
{
	int x;
    x = 1000000 / t;
    TA0CCR0 = x; // ex. t = 10 --> (1000000 [Hz]) / 100000 = 10 Hz
    TA0CCTL0 = CCIE; // capture compare interrupt enabled
    
    // Duty cycle timer
    TA1CCR0 = 100;
    TA1CTL = TASSEL_2 + MC_1 + TACLR; 
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TA0CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TACLR clears timer A0 register
	TA0CTL = TASSEL_2 + MC_1 + TACLR; // Begin timer right away
	
    P1IFG &= ~BIT3;   // Clear P1.3 interrupt flag
    P1IE &= ~BIT3;  // Disable interrupts to prevent false alarm

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{

	// This switch is the logic for determining the status of the button
	// On press, the case 0 loop is entered, and on release the case 1 loop is entered
	
	switch(state) {
	
	case 0:
	    if(dutycycle < 100) {
	        dutycycle = dutycycle + 10;
	    }
        else dutycycle = 0;     
		P1OUT ^= BIT6; // Blink green LED
		P1IES &= ~BIT3; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P1OUT ^= BIT6; // Blink green LED
		P1IFG &= ~BIT3; // Clear flag
		P1IES |= BIT3; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P1IE |= BIT3; // Reenable interrupts
	TA0CTL &= ~ TASSEL_2; // Stop timer
	TA0CTL |= TACLR; // Clear Timer
	
}

### MSP430FR2311
//---------------------------------------------------------------------------------------

// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

volatile int state = 0;
volatile int dutycycle = 50; // duty cycle initialized at 50%

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	// Disables default high-impedance mode
	PM5CTL0 &= ~LOCKLPM5;

	// Button configuration
    P1DIR = BIT0; // Set P1.0 as output
	P2DIR = BIT0; // Set P2.0 as output
    P1REN |= BIT1; // Connects the on-board resistor to P1.1
    P1OUT = BIT1; // Sets up P1.1 as pull-up resistor
    
	// Interrupt Configuration
    P1IES |= BIT1; // Interrupts on button release LO TO HI
    P1IE |= BIT1; // Enable interrupt on button pin
    P1IFG &= ~BIT1; // Clear interrupt flag

	// Timer frequency of 100 Hz --> 10 ms intervals
    frequencyCalc(100);    // initialize timer to 100Hz

    __enable_interrupt(); // MUST BE ENABLED IN ADDITION TO GIE
    __bis_SR_register(GIE); // enable global interrupts
    
	// Compare the current value of the timer in the A1 register
	// with the duty cycle to determine the rate of LED flicker
    while (1) {
        if(TB1R <= dutycycle) 
            P1OUT ^= BIT0;
        else if (TB1R > dutycycle) 
            P1OUT &= ~BIT0;
    }

}

// Sets up the timer compare value to 
void frequencyCalc(int t)
{
	int x;
    x = 1000000 / t;
    TB0CCR0 = x; // ex. t = 10 --> (1000000 [Hz]) / 100000 = 10 Hz
    TB0CCTL0 = CCIE; // capture compare interrupt enabled
    
    // Duty cycle timer
    TB1CCR0 = 100;
    TB1CTL = TBSSEL_2 + MC_1 + TBCLR; 
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TB0CTL = Timer A0 chosen for use
    // TBSSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TBCLR clears timer A0 register
	TB0CTL = TBSSEL_2 + MC_1 + TBCLR; // Begin timer right away
	
    P1IFG &= ~BIT1;   // Clear P1.1 interrupt flag
    P1IE &= ~BIT1;  // Disable interrupts to prevent false alarm

}

// Interrupt subroutine
// Called when timer reaches TB0CCR0
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B0(void)
{

	// This switch is the logic for determining the status of the button
	// On press, the case 0 loop is entered, and on release the case 1 loop is entered
	
	switch(state) {
	
	case 0:
	    if(dutycycle < 100) {
	        dutycycle = dutycycle + 10;
	    }
        else dutycycle = 0;     
		P2OUT ^= BIT0; // Blink green LED
		P1IES &= ~BIT1; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P2OUT ^= BIT1; // Blink green LED
		P1IFG &= ~BIT1; // Clear flag
		P1IES |= BIT1; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P1IE |= BIT1; // Reenable interrupts
	TB0CTL &= ~ TBSSEL_2; // Stop timer
	TB0CTL |= TBCLR; // Clear Timer
	
}

### MSP430F5529
//---------------------------------------------------------------------------------------

// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

volatile int state = 0;
volatile int dutycycle = 50; // duty cycle initialized at 50%

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

	// Button configuration
    P1DIR |= BIT0; // Set P1.0 as output
	P4DIR |= BIT7; // Set P4.7 as output
    P1REN |= BIT1; // Connects the on-board resistor to P1.1
    P1OUT = BIT1; // Sets up P1.3 as pull-up resistor
    
	// Interrupt Configuration
    P1IES |= BIT1; // Interrupts on button release LO TO HI
    P1IE |= BIT1; // Enable interrupt on button pin
    P1IFG &= ~BIT1; // Clear interrupt flag

	// Timer frequency of 100 Hz --> 10 ms intervals
    frequencyCalc(100);    // initialize timer to 100Hz

    __enable_interrupt(); // MUST BE ENABLED IN ADDITION TO GIE
    __bis_SR_register(GIE); // enable global interrupts
    
	// Compare the current value of the timer in the A1 register
	// with the duty cycle to determine the rate of LED flicker
    while (1) {
        if(TA1R <= dutycycle) 
            P1OUT ^= BIT0;
        else if (TA1R > dutycycle) 
            P1OUT &= ~BIT0;
    }

}

// Sets up the timer compare value to 
void frequencyCalc(int t)
{
	int x;
    x = 1000000 / t;
    TA0CCR0 = x; // ex. t = 10 --> (1000000 [Hz]) / 100000 = 10 Hz
    TA0CCTL0 = CCIE; // capture compare interrupt enabled
    
    // Duty cycle timer
    TA1CCR0 = 100;
    TA1CTL = TASSEL_2 + MC_1 + TACLR; 
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TA0CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TACLR clears timer A0 register
	TA0CTL = TASSEL_2 + MC_1 + TACLR; // Begin timer right away
	
    P1IFG &= ~BIT1;   // Clear P1.1 interrupt flag
    P1IE &= ~BIT1;  // Disable interrupts to prevent false alarm

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{

	// This switch is the logic for determining the status of the button
	// On press, the case 0 loop is entered, and on release the case 1 loop is entered
	
	switch(state) {
	
	case 0:
	    if(dutycycle < 100) {
	        dutycycle = dutycycle + 10;
	    }
        else dutycycle = 0;     
		P4OUT ^= BIT7; // Blink green LED
		P1IES &= ~BIT1; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P4OUT ^= BIT7; // Blink green LED
		P1IFG &= ~BIT1; // Clear flag
		P1IES |= BIT1; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P1IE |= BIT1; // Reenable interrupts
	TA0CTL &= ~ TASSEL_2; // Stop timer
	TA0CTL |= TACLR; // Clear Timer
	
}

### MSP430FR5994
//---------------------------------------------------------------------------------------

// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

volatile int state = 0;
volatile int dutycycle = 50; // duty cycle initialized at 50%

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	// Disables default high-impedance mode
	PM5CTL0 &= ~LOCKLPM5;

	// Button configuration
    P1DIR = BIT0 + BIT1; // Set P1.0 and P1.1 as output
    P5REN |= BIT5; // Connects the on-board resistor to P5.5
    P5OUT = BIT5; // Sets up P5.5 as pull-up resistor
    
	// Interrupt Configuration
    P5IES |= BIT5; // Interrupts on button release LO TO HI
    P5IE |= BIT5; // Enable interrupt on button pin
    P5IFG &= ~BIT5; // Clear interrupt flag

	// Timer frequency of 100 Hz --> 10 ms intervals
    frequencyCalc(100);    // initialize timer to 100Hz

    __enable_interrupt(); // MUST BE ENABLED IN ADDITION TO GIE
    __bis_SR_register(GIE); // enable global interrupts
    
	// Compare the current value of the timer in the A1 register
	// with the duty cycle to determine the rate of LED flicker
    while (1) {
        if(TA1R <= dutycycle) 
            P1OUT ^= BIT0;
        else if (TA1R > dutycycle) 
            P1OUT &= ~BIT0;
    }

}

// Sets up the timer compare value to 
void frequencyCalc(int t)
{
	int x;
    x = 1000000 / t;
    TA0CCR0 = x; // ex. t = 10 --> (1000000 [Hz]) / 100000 = 10 Hz
    TA0CCTL0 = CCIE; // capture compare interrupt enabled
    
    // Duty cycle timer
    TA1CCR0 = 100;
    TA1CTL = TASSEL_2 + MC_1 + TACLR; 
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT5_VECTOR
__interrupt void PORT_5(void)
{

    // TA0CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TACLR clears timer A0 register
	TA0CTL = TASSEL_2 + MC_1 + TACLR; // Begin timer right away
	
    P5IFG &= ~BIT5;   // Clear P5.5 interrupt flag
    P5IE &= ~BIT5;  // Disable interrupts to prevent false alarm

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{

	// This switch is the logic for determining the status of the button
	// On press, the case 0 loop is entered, and on release the case 1 loop is entered
	
	switch(state) {
	
	case 0:
	    if(dutycycle < 100) {
	        dutycycle = dutycycle + 10;
	    }
        else dutycycle = 0;     
		P1OUT ^= BIT1; // Blink green LED
		P5IES &= ~BIT5; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P1OUT ^= BIT1; // Blink green LED
		P5IFG &= ~BIT5; // Clear flag
		P5IES |= BIT5; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P5IE |= BIT5; // Reenable interrupts
	TA0CTL &= ~ TASSEL_2; // Stop timer
	TA0CTL |= TACLR; // Clear Timer
	
}

### MSP430FR6989
//---------------------------------------------------------------------------------------

// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

volatile int state = 0;
volatile int dutycycle = 50; // duty cycle initialized at 50%

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	// Disables default high-impedance mode
	PM5CTL0 &= ~LOCKLPM5;

	// Button configuration
    P1DIR |= BIT0; // Set P1.0 as output
	P9DIR |= BIT7; // Set P9.7 as output
    P1REN |= BIT1; // Connects the on-board resistor to P1.1
    P1OUT = BIT1; // Sets up P1.3 as pull-up resistor
    
	// Interrupt Configuration
    P1IES |= BIT1; // Interrupts on button release LO TO HI
    P1IE |= BIT1; // Enable interrupt on button pin
    P1IFG &= ~BIT1; // Clear interrupt flag

	// Timer frequency of 100 Hz --> 10 ms intervals
    frequencyCalc(100);    // initialize timer to 100Hz

    __enable_interrupt(); // MUST BE ENABLED IN ADDITION TO GIE
    __bis_SR_register(GIE); // enable global interrupts
    
	// Compare the current value of the timer in the A1 register
	// with the duty cycle to determine the rate of LED flicker
    while (1) {
        if(TA1R <= dutycycle) 
            P1OUT ^= BIT0;
        else if (TA1R > dutycycle) 
            P1OUT &= ~BIT0;
    }

}

// Sets up the timer compare value to 
void frequencyCalc(int t)
{
	int x;
    x = 1000000 / t;
    TA0CCR0 = x; // ex. t = 10 --> (1000000 [Hz]) / 100000 = 10 Hz
    TA0CCTL0 = CCIE; // capture compare interrupt enabled
    
    // Duty cycle timer
    TA1CCR0 = 100;
    TA1CTL = TASSEL_2 + MC_1 + TACLR; 
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TA0CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TACLR clears timer A0 register
	TA0CTL = TASSEL_2 + MC_1 + TACLR; // Begin timer right away
	
    P1IFG &= ~BIT1;   // Clear P1.1 interrupt flag
    P1IE &= ~BIT1;  // Disable interrupts to prevent false alarm

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{

	// This switch is the logic for determining the status of the button
	// On press, the case 0 loop is entered, and on release the case 1 loop is entered
	
	switch(state) {
	
	case 0:
	    if(dutycycle < 100) {
	        dutycycle = dutycycle + 10;
	    }
        else dutycycle = 0;     
		P9OUT ^= BIT7; // Blink green LED
		P1IES &= ~BIT1; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P9OUT ^= BIT7; // Blink green LED
		P1IFG &= ~BIT1; // Clear flag
		P1IES |= BIT1; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P1IE |= BIT1; // Reenable interrupts
	TA0CTL &= ~ TASSEL_2; // Stop timer
	TA0CTL |= TACLR; // Clear Timer
	
}

## Extra work for MSP430G2553
//---------------------------------------------------------------------------------------

The logarithmic scaling was implemented in the timer interrupt, the same place it was
done linearly before. Now, however, it uses a linear variable duty count to track
increment level (0-9) which frees up the dutycycle variable to increase non-linearly.

The dutycycle increases exponentially to counter the logarithmic viewing of our eyes
thus producing a *VISUALLY* linear increase. An approximation exponential was chosen
to be e^(x-3) where x exists in increments of 0.3 with domain [0.3, 3] because at x = 0, 
the brightness is close enough to zero but and at x = 3 the value is 1 which can be 
considered full brightness. From here, it was possible to tweak the model to simulate
an exponential increase. 

This approximation was chosen over a real exponential function because it significantly
reduces calculation time and can deal solely with integers.

// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

volatile int state = 0;
volatile int dutycycle = 50; // duty cycle initialized at 50%
volatile int dutycount = 5; // keeps track of exponential duty cycle

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

	// Button configuration
    P1DIR = BIT0 + BIT6; // Set P1.0 and BIT6 as output
    P1REN |= BIT3; // Connects the on-board resistor to P1.3
    P1OUT = BIT3; // Sets up P1.3 as pull-up resistor
    
	// Interrupt Configuration
    P1IES |= BIT3; // Interrupts on button release LO TO HI
    P1IE |= BIT3; // Enable interrupt on button pin
    P1IFG &= ~BIT3; // Clear interrupt flag

	// Timer frequency of 100 Hz --> 10 ms intervals
    frequencyCalc(100);    // initialize timer to 100Hz

    __enable_interrupt(); // MUST BE ENABLED IN ADDITION TO GIE
    __bis_SR_register(GIE); // enable global interrupts
    
	// Compare the current value of the timer in the A1 register
	// with the duty cycle to determine the rate of LED flicker
    while (1) {
        if(TA1R <= dutycycle) 
            P1OUT ^= BIT0;
        else if (TA1R > dutycycle) 
            P1OUT &= ~BIT0;
    }

}

// Sets up the timer compare value to 
void frequencyCalc(int t)
{
	int x;
    x = 1000000 / t;
    TA0CCR0 = x; // ex. t = 10 --> (1000000 [Hz]) / 100000 = 10 Hz
    TA0CCTL0 = CCIE; // capture compare interrupt enabled
    
    // Duty cycle timer
    TA1CCR0 = 100;
    TA1CTL = TASSEL_2 + MC_1 + TACLR; 
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TA0CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TACLR clears timer A0 register
	TA0CTL = TASSEL_2 + MC_1 + TACLR; // Begin timer right away
	
    P1IFG &= ~BIT3;   // Clear P1.3 interrupt flag
    P1IE &= ~BIT3;  // Disable interrupts to prevent false alarm

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{

	// This switch is the logic for determining the status of the button
	// On press, the case 0 loop is entered, and on release the case 1 loop is entered
	
	switch(state) {
	
	case 0:
	
		// LOGARITHMIC CHANGE
		// The logarithmic scale is approximated according to the equation y = e^(x-3) * 100
		// where x ranges from 0 to 3 with a step size of 0.3 to produce 10 intervals.
		// Tweaking on the fringe cases (case 0 and case 9) was done to make the model
		// more logarithmic in nature and consistent in terms of viewing brightness.
	    if(dutycount < 10) {
			switch (dutycount) {
				case 0:
					// e^(0.3-3) * 100 = 6
					dutycycle = dutycycle + 2; // changed reduced by 4 here (from 6)
					dutycount++;
					break;
				case 1:
					// e^(0.6-3) * 100 = 9
					dutycycle = dutycycle + 3; // 9-6=3
					dutycount++;
					break;
				case 2:
					// e^(0.9-3) * 100 = 12
					dutycycle = dutycycle + 3; // 12-9=3
					dutycount++;
					break;
				case 3:
					// e^(1.2-3) * 100 = 17
					dutycycle = dutycycle + 5; // 17-12=5
					dutycount++;
					break;
				case 4:
					// e^(1.5-3) * 100 = 22
					dutycycle = dutycycle + 5; // 22-17=5
					dutycount++;
					break;
				case 5:
					// e^(1.8-3) * 100 = 30
					dutycycle = dutycycle + 8; // 30-22=8
					dutycount++;
					break;
				case 6:
					// e^(2.1-3) * 100 = 41
					dutycycle = dutycycle + 11; // 41-30=11
					dutycount++;
					break;
				case 7:
					// e^(2.4-3) * 100 = 55
					dutycycle = dutycycle + 14; // 55-41=14
					dutycount++;
					break;
				case 8:
					// e^(2.7-3) * 100 = 74
					dutycycle = dutycycle + 19; // 74-55=19
					dutycount++;
					break;
				case 9:
					// e^(3-3) * 100 = 100 = 26
					dutycycle = dutycycle + 30; // change increased by 4 here (from 26)
					dutycount++;
					break;
			}
	    }
        else {
			dutycycle = 0;
			dutycount = 0;
		}
		
		P1OUT ^= BIT6; // Blink green LED
		P1IES &= ~BIT3; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P1OUT ^= BIT6; // Blink green LED
		P1IFG &= ~BIT3; // Clear flag
		P1IES |= BIT3; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P1IE |= BIT3; // Reenable interrupts
	TA0CTL &= ~ TASSEL_2; // Stop timer
	TA0CTL |= TACLR; // Clear Timer
	
}