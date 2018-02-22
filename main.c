#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define LED_1 1<<PC0	// Zdefiniowanie pinów.
#define LED_2 1<<PC1
#define LED_3 1<<PC2

volatile static uint16_t licznik; 		// Zmienna uzywane zarówno w petli g³ównej jak i w przerwaniu.
uint16_t PWM_1, PWM_2, PWM_3;

uint16_t zmiana_pwm_1; 		// Zmienne s³u¿¹ce do pobierania kolejnych wartoœci z tablicy
uint16_t zmiana_pwm_2;
uint16_t zmiana_pwm_3;

uint8_t licz_w_gore_1=1; 	// Zmienne s³u¿¹ce do wygenerowania przesuniêcia fazowego
uint8_t licz_w_gore_2=0;
uint8_t licz_w_gore_3=0;
uint8_t zaladuj_raz=1;

uint16_t rozdzielczosc=1000; // Zmienne s³u¿¹ce do wygenerowania tablicy z wartoœciami PWM dla sinusoidalnego kszta³tu sygna³u
uint16_t PWM_tablica[1000];
uint16_t N;
uint8_t stworz_tablice=1;
uint8_t dn;

int main (void)
{
	TCCR0|= (1<<WGM01); // Ustawienie trybu licznika na CTC
	TCCR0|= (1<<CS00);	// Preskaler =1
	TIMSK|= (1<<OCIE0);	// Zezwolenie na wykonanie przerwania od compare match
	OCR0=5; 			// Rejestr porównania, licznik zlicza do osi¹gniêcia tej wartoœci.
						// Przerwanie generowane z czêstotliwoœci¹ = 8Mhz/preskaler=1/OCRO=x
						// Zmiany OCR0 wp³ywa na czêstotliwoœc sinusoidy

	DDRC |=  LED_1|LED_2|LED_3; 	// Piny jako wyjœcia.
	PORTC|=  LED_1|LED_2|LED_3; 	// Podanie stanu wysokiego, ledy wy³¹czone w momencie w³¹czenie mikrokontroloera (dla led pod³aczonych katod¹ do pinów).
	sei(); 							// Uruchomienie przerwañ.

	N=rozdzielczosc-1; 					  // Numer ostatniego elementu tablicy
	dn=rozdzielczosc/(rozdzielczosc*0.5); // Przyrost wartoœci PWM

	while(1)
	{
		//Utworzenie tablicy wartoœci PWM dla sinusoidalnego kszta³tu sygna³u.
		if(stworz_tablice) // Tablica zostania wykonana tylko raz, przy pierwszym przebiegu programu
		{
			uint16_t i=0;
			for (i=0;i<1;i++){PWM_tablica[i]=(rozdzielczosc*0.5);} 				 // Pocz¹tek sinusoidy
			for (i=1;i<(N*0.25);i++){PWM_tablica[i]=PWM_tablica[i-1]+dn;} 		 // Narastanie sinusoidy do amplitudy dodatniej
			for (i=N*0.25;i<(N*0.25)+1;i++){PWM_tablica[i]=N;} 					 // Amplituda dodatnia
			for (i=(N*0.25)+1;i<(N*0.75);i++){PWM_tablica[i]=PWM_tablica[i-1]-dn;if(PWM_tablica[i]<0)PWM_tablica[i]=PWM_tablica[i-1];} // Spadek do amplitudy ujemnej
			for (i=N*0.75;i<(N*0.75)+1;i++){PWM_tablica[i]=0;} 					 // Amplituda ujemna
			for (i=(N*0.75)+1;i<=N;i++){PWM_tablica[i]=PWM_tablica[i-1]+dn;} 	 // Narastanie sinusoidy do pocz¹tku
			if (i>N){stworz_tablice=0;}
		}

		if (zaladuj_raz==1)// Przesuniêcia fazowe
		{
			if(zmiana_pwm_1>(N*0.33)){licz_w_gore_2=1;}				 // Przesuniêcie drugiej fazy 0 120 stopni wzglêdem pierwszej fazy.
			if(zmiana_pwm_1>(N*0.66)){licz_w_gore_3=1;zaladuj_raz=0;}// Przesuniêcie trzeciej fazy 0 240 stopni wzglêdem pierwszej fazy.
		}


		if (licznik>N) // Cykliczna zmiana elementów tablicy
		{
			licznik=0;

			if (licz_w_gore_1==1)//Zmiany wartoœci PWM dla pierwszej sinusoidy.
			{zmiana_pwm_1++;
			if(zmiana_pwm_1>N)zmiana_pwm_1=0;}

			if (licz_w_gore_2==1)//Zmiany wartoœci PWM dla drugiej sinusoidy.
			{zmiana_pwm_2++;
			if(zmiana_pwm_2>N)zmiana_pwm_2=0;}

			if (licz_w_gore_3==1)//Zmiany wartoœci PWM dla trzeciej sinusoidy.
			{zmiana_pwm_3++;
			if(zmiana_pwm_3>N)zmiana_pwm_3=0;}
		}

		PWM_1=PWM_tablica[zmiana_pwm_1];  // Przypisanie zawartoœci kolejnych elementów tablicy do zmiennych PWM
		PWM_2=PWM_tablica[zmiana_pwm_2];
		PWM_3=PWM_tablica[zmiana_pwm_3];

		if(licznik>=PWM_1)PORTC|=(LED_1); else PORTC&=~(LED_1); // Zmiana stanów na wyjœciach pinów
		if(licznik>=PWM_2)PORTC|=(LED_2); else PORTC&=~(LED_2);
		if(licznik>=PWM_3)PORTC|=(LED_3); else PORTC&=~(LED_3);
	}
}

ISR(TIMER0_COMP_vect) // Wektor przerwania od compare match.
{
	licznik++; // Instrukcja obs³ugi przerwania.
}











