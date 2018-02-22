#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define LED_1 1<<PC0	// Zdefiniowanie pin�w.
#define LED_2 1<<PC1
#define LED_3 1<<PC2

volatile static uint16_t licznik; 		// Zmienna uzywane zar�wno w petli g��wnej jak i w przerwaniu.
uint16_t PWM_1, PWM_2, PWM_3;

uint16_t zmiana_pwm_1; 		// Zmienne s�u��ce do pobierania kolejnych warto�ci z tablicy
uint16_t zmiana_pwm_2;
uint16_t zmiana_pwm_3;

uint8_t licz_w_gore_1=1; 	// Zmienne s�u��ce do wygenerowania przesuni�cia fazowego
uint8_t licz_w_gore_2=0;
uint8_t licz_w_gore_3=0;
uint8_t zaladuj_raz=1;

uint16_t rozdzielczosc=1000; // Zmienne s�u��ce do wygenerowania tablicy z warto�ciami PWM dla sinusoidalnego kszta�tu sygna�u
uint16_t PWM_tablica[1000];
uint16_t N;
uint8_t stworz_tablice=1;
uint8_t dn;

int main (void)
{
	TCCR0|= (1<<WGM01); // Ustawienie trybu licznika na CTC
	TCCR0|= (1<<CS00);	// Preskaler =1
	TIMSK|= (1<<OCIE0);	// Zezwolenie na wykonanie przerwania od compare match
	OCR0=5; 			// Rejestr por�wnania, licznik zlicza do osi�gni�cia tej warto�ci.
						// Przerwanie generowane z cz�stotliwo�ci� = 8Mhz/preskaler=1/OCRO=x
						// Zmiany OCR0 wp�ywa na cz�stotliwo�c sinusoidy

	DDRC |=  LED_1|LED_2|LED_3; 	// Piny jako wyj�cia.
	PORTC|=  LED_1|LED_2|LED_3; 	// Podanie stanu wysokiego, ledy wy��czone w momencie w��czenie mikrokontroloera (dla led pod�aczonych katod� do pin�w).
	sei(); 							// Uruchomienie przerwa�.

	N=rozdzielczosc-1; 					  // Numer ostatniego elementu tablicy
	dn=rozdzielczosc/(rozdzielczosc*0.5); // Przyrost warto�ci PWM

	while(1)
	{
		//Utworzenie tablicy warto�ci PWM dla sinusoidalnego kszta�tu sygna�u.
		if(stworz_tablice) // Tablica zostania wykonana tylko raz, przy pierwszym przebiegu programu
		{
			uint16_t i=0;
			for (i=0;i<1;i++){PWM_tablica[i]=(rozdzielczosc*0.5);} 				 // Pocz�tek sinusoidy
			for (i=1;i<(N*0.25);i++){PWM_tablica[i]=PWM_tablica[i-1]+dn;} 		 // Narastanie sinusoidy do amplitudy dodatniej
			for (i=N*0.25;i<(N*0.25)+1;i++){PWM_tablica[i]=N;} 					 // Amplituda dodatnia
			for (i=(N*0.25)+1;i<(N*0.75);i++){PWM_tablica[i]=PWM_tablica[i-1]-dn;if(PWM_tablica[i]<0)PWM_tablica[i]=PWM_tablica[i-1];} // Spadek do amplitudy ujemnej
			for (i=N*0.75;i<(N*0.75)+1;i++){PWM_tablica[i]=0;} 					 // Amplituda ujemna
			for (i=(N*0.75)+1;i<=N;i++){PWM_tablica[i]=PWM_tablica[i-1]+dn;} 	 // Narastanie sinusoidy do pocz�tku
			if (i>N){stworz_tablice=0;}
		}

		if (zaladuj_raz==1)// Przesuni�cia fazowe
		{
			if(zmiana_pwm_1>(N*0.33)){licz_w_gore_2=1;}				 // Przesuni�cie drugiej fazy 0 120 stopni wzgl�dem pierwszej fazy.
			if(zmiana_pwm_1>(N*0.66)){licz_w_gore_3=1;zaladuj_raz=0;}// Przesuni�cie trzeciej fazy 0 240 stopni wzgl�dem pierwszej fazy.
		}


		if (licznik>N) // Cykliczna zmiana element�w tablicy
		{
			licznik=0;

			if (licz_w_gore_1==1)//Zmiany warto�ci PWM dla pierwszej sinusoidy.
			{zmiana_pwm_1++;
			if(zmiana_pwm_1>N)zmiana_pwm_1=0;}

			if (licz_w_gore_2==1)//Zmiany warto�ci PWM dla drugiej sinusoidy.
			{zmiana_pwm_2++;
			if(zmiana_pwm_2>N)zmiana_pwm_2=0;}

			if (licz_w_gore_3==1)//Zmiany warto�ci PWM dla trzeciej sinusoidy.
			{zmiana_pwm_3++;
			if(zmiana_pwm_3>N)zmiana_pwm_3=0;}
		}

		PWM_1=PWM_tablica[zmiana_pwm_1];  // Przypisanie zawarto�ci kolejnych element�w tablicy do zmiennych PWM
		PWM_2=PWM_tablica[zmiana_pwm_2];
		PWM_3=PWM_tablica[zmiana_pwm_3];

		if(licznik>=PWM_1)PORTC|=(LED_1); else PORTC&=~(LED_1); // Zmiana stan�w na wyj�ciach pin�w
		if(licznik>=PWM_2)PORTC|=(LED_2); else PORTC&=~(LED_2);
		if(licznik>=PWM_3)PORTC|=(LED_3); else PORTC&=~(LED_3);
	}
}

ISR(TIMER0_COMP_vect) // Wektor przerwania od compare match.
{
	licznik++; // Instrukcja obs�ugi przerwania.
}











