/* Tomasz Widomski zad 8 27.06.2020
 * 8)Napisać program jednowątkowy który wykonuje obliczenia maksymalnej
 * co do modułuwartości   własnej   i   wektora   własnego   macierzy
 * kwadratowej   symetrycznej   metodąpotęgową.
 * Metoda   polega   na   tym   że   dla   danej   macierzy   i
 * wektora   startowegojednostkowego (wszystkie współrzędne równe jeden)
 * wykonuje się mnożenie wielokrotniewektora przez macierz: v=A*v.
 * Otrzymany nowy wektor   v   jest normalizowany w tensposób że
 * wszystkie jego składowe są dzielone przez maksymalną co do modułu
 * składowątego wektora. Tak znormalizowany wektor jest ponownie mnożony
 * przez macierz A i procedura jest powtarzana dopóki zmiana składowej
 * maksymalnej wektora pomiędzydwoma ostatnimi iteracjami jest bardzo mała.
 * Cały algorytm można zapisać w pseudokodzienastępująco:
 * ...  // inicjalizacja danych
 * p0: d =1
 * p0: m1=0
 * p1: while ( d<0.000001)
 * p2:v=A*v
 * p3:m2=max(v)
 * p4:d=m2-m1
 * p5m1=m2
 * p6for i =1,n
 * p7v[i] =v[i] /m1
 * p8: wypisz(m1)
 * Korzystając z interfejsu OpenMP stworzyć równoległą wersję tego programu.
 * Policzyć  efektywność tego programu równoległego podczas jego wykonywania
 * na   dostępnej  maszynie wieloprocesorowej. Do obliczenia efektywności
 * wykorzystać odpowiedni wzór z wykładu. Jako złożoność czasową przyjąć
 * wartość czasu wykonywania się algorytmu (nie całego programu).
 * Jako złożoność procesorową przyjąć liczbę   fizycznych   procesorów  (rdzeni)
 * dostępnych na danej maszynie. Wyniki obliczeń oraz sekwencyjną i równoległą wersję
 * programu przedstawić jako rozwiązanie zadania. Wyniki obliczeń należy przedstawić
 * jako komentarz, w pliku zawierającym kod źródłowy programu.
 *
 * */


//Wersja rownolegla programu:
#include <cstdio>
#include <omp.h>
#include <chrono>
using namespace std::chrono;

#define liczba_watkow  4


double **stworzTablicePomocnicza(int tablicy);
double **stworzMacierz(int wielkoscTablicy);
double **stworzWektorStartowy(int wielkoscTablicy);

int main(){
    int wielkoscTablicy = 40000;

//    printf("Podaj wielkosc tablicy: ");
//    scanf ("%d",&a);
//    wielkoscTablicy = a;

    //inicjalizacja danych
    double** A = stworzMacierz(wielkoscTablicy);
    double** v = stworzWektorStartowy(wielkoscTablicy);
    double** tabPom = stworzTablicePomocnicza(wielkoscTablicy);

    //p0: d=1; m1=0
    double d = 1.0;
    double m1, m2 = 0.0;



    // p1: while
    while(d > 0.000001){
        // p2: v = A*v
        int i, k;

        auto start = high_resolution_clock::now();

#pragma omp parallel for shared(tabPom, v, A, wielkoscTablicy) private(i, k) num_threads(liczba_watkow)
        for(i = 0; i < wielkoscTablicy; i++)
        {
            tabPom[i][0] = 0;
            for(k = 0; k < wielkoscTablicy; k++)
            {
                tabPom[i][0] += A[i][k] * v[k][0];
            }
        }

#pragma omp parallel for shared(tabPom, v, wielkoscTablicy) private(i) num_threads(liczba_watkow)
        for(i = 0; i < wielkoscTablicy; i++)
        {
            v[i][0] = tabPom[i][0];
        }
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        printf("Time taken by function: %lld miliseconds\n", static_cast<long long int>(duration.count())/10000);


        // p3: m2 = max(v)
#pragma omp parallel for shared(m2, wielkoscTablicy, v, tabPom) private(i) num_threads(liczba_watkow)
        for(i = 0; i < wielkoscTablicy ; i++)
        {
            v[i][0] = tabPom[i][0];
            if(m2 < v[i][0])
            {
#pragma omp critical
                m2=v[i][0];
            }
        }

        // p4: d = |m2 - m1|
        d = m2 > m1 ? m2 - m1 : m1 - m2;

        // p5: m1=m2
        m1 = m2;
        m2 = 0;
        //p6: for i =1,n
#pragma omp parallel for shared(tabPom, v, wielkoscTablicy) private(i) num_threads(liczba_watkow)
        for( i = 0; i < wielkoscTablicy ; i++)
        {
            //p7: v[i] = v[i] / m1
            v[i][0] /= m1;
        }
#pragma omp barrier
    }


//  p8: wypisz(m1)
    printf("m1: %f", m1);

    for (int i = 0; i < wielkoscTablicy; i++)
    {
        delete [] v[i];
        delete [] tabPom[i];
        delete [] A[i];
    }

    delete [] A;
    delete [] v;
    delete [] tabPom;

    return 0;
}





double **stworzTablicePomocnicza(int wielkoscTablicy)
{
    double** tab = new double* [wielkoscTablicy];

    for(int i = 0; i < wielkoscTablicy ; i++){
        tab[i] = new double [1];
        for(int j = 0; j < 1 ; j++)
        {
            tab[i][j] = 0;
        }
    }
    return tab;
}

double** stworzMacierz(int wielkoscTablicy){
    double** tab = new double* [wielkoscTablicy];

    for(int i = 0; i < wielkoscTablicy ; i++)
    {
        tab[i] = new double [wielkoscTablicy];
        for(int j = 0; j < wielkoscTablicy ; j++)
        {
            //initial values;
            tab[i][j] = 1;
        }
    }

////tablica testowa
//    tab[0][0] = 1;
//    tab[0][1] = 3;
//    tab[0][2] = 7;
//    tab[1][0] = 3;
//    tab[1][1] = 4;
//    tab[1][2] = 8;
//    tab[2][0] = 7;
//    tab[2][1] = 8;
//    tab[2][2] = 9;


//    //Wczytywanie wlasnych wartosci z klawiatury
//    double val = 0.0;
//    for(int i = 0 ; i < wielkoscTablicy ; i++){
//         for(int j = 0 ; j < wielkoscTablicy ; j++){
//             printf("Podaj wartosc elementu &d, &d\n", i, j);
//             scanf ("%f",&val);
//             tab[i][j] = val;
//
//         }
//     }

    return tab;

}

double** stworzWektorStartowy(int wielkoscTablicy)
{
    double ** tab = new double* [wielkoscTablicy];
    for(int i = 0; i < wielkoscTablicy ; i++)
    {
        tab[i] = new double [1];
        tab[i][0] = 1;
    }
    return tab;
}


//Efektywnosc programu rownloeglego

/*
 Zdecydowalem sie porownac efektywnosc algorytmu dla algorytmu mnozenia macierzy, czyli punktu p2: v= A*v
 Pozostale fragmenty kodu rowniez mozna bylo obliczyc w identyczny sposob, jednak ten fragment wydawal mi sie najbardziej interesujacy do wykonania tego testu.

 Test czasu wykonania algorytmu przeprowadzilem za pomoca biblioteki chrono, sposob pomiaru zostal zawarty w niezmienionej formie w kodzie obu programow.
 Wyniki testu sa usrednione, test przeprowadzilem uruchamiajac parukrotnie obie aplikacje, a nastepnie spisujac wyniki czasu wykonania algorytmu.
 Test odbyl sie na macierzy jednostkowej (zawierajacej same jedynki) o wymiarach 40000 x 40000.


 E(p,n) = S(p,n)/p - wzor na efektywnosc algorytmu, gdzie
 p - liczba procesorow
 n - rozmiar zadania
 S(p,n) = T*(1,n)/T(p,n) - przyspieszenie algorytmu sekwencyjnego
 T*(1,n) - zlozonosc obliczeniowa(zlozonosc czasowa najszybszego algorytmu
sekwencyjnego rozwiozujacego dany problem)
 T(p,n) - zlozonosc czasowa algorytmu rownleglego wykonywania na jednym procesorze


Dane:
p - 4
n - 40000
T*(1,n) = 330ms
T(p,n)  = 990ms

T*(1,n) / T(p,n) = 330/990 = 1/3
 E(p,n) = 1/3/4 = 1/12
 Odp. Efektywnosc algorytmu wynosi 1/12, czyli okolo 0.083% wzgledem 100% podczas
 wykonywania tego samego fragmentu kodu przez jeden watek.


 *
 */
