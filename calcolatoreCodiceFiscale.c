#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

/* PROGRAMMA: Calcolatore del codice fiscale per persone fisiche nate in Italia
 * AUTORE: Lorenzo Porta - ITT "G. Fauser" - Novara
 * ULTIMA MODIFICA: 03/11/2024 - 12:47
 */

/* ELENCO CODICI ERRORE RESTITUITI:
 *      0 - Esecuzione conclusa con successo;
 *      1 - Errore nell'allocazione dinamica;
 *      2 - Luogo di nascita non presente nel file codiciCatastali.csv
 */

// Costanti generiche
#define ANNO_MIN 1900
#define DIV_CHAR 59 // = ';'
#define NUM_MESI 12

// Lunghezze delle stringhe che conterrano i dati della persona
#define LEN_NOME 20
#define LEN_COGNOME 20
#define LEN_LUOGO_NASCITA 40

// Lunghezze minime per la validazione dei parametri inseriti dall'utente
#define LEN_MIN_NOME 3
#define LEN_MIN_COGNOME 2
#define LEN_MIN_LUOGO_NASCITA 2

// Lunghezze delle singole codifiche
#define LEN_COD_NOME 3
#define LEN_COD_COGNOME 3
#define LEN_COD_DN 5
#define LEN_COD_CATASTALE 4
#define LEN_CF 16

// Lettere corrispondenti ai mesi per la codifica della data di nascita
const char MESI[NUM_MESI + 1] = "_ABCDEHLMPRST";

// Alfabeti per la determinazione del CIN
const char CARATTERI[36] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int VALORE_CARATTERI_DISPARI[36] = {1, 0, 5, 7, 9, 13, 15, 17, 19,
                                          21, 1, 0, 5, 7, 9, 13, 15,
                                          17, 19, 21, 2, 4, 18, 20, 11,
                                          3, 6, 8, 12, 14, 16, 10, 22,
                                          25, 24, 23};
const int VALORE_CARATTERI_PARI[36] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
                                       1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                       11, 12, 13, 14, 15, 16, 17, 18, 19,
                                       20, 21, 22, 23, 24, 25};
const char CARATTERI_RESTO[26] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

typedef struct DATA {
    int giorno;
    int mese;
    int anno;
}data;

// Controlla se il carattere passato come parametro è una vocale italiana (secondo tabella ASCII standard)
bool IsVocale(char c){
    if(c == 'a' || c == 'A' || c == 'e' || c == 'E' || c == 'i' || c == 'I' || c == 'o' || c == 'O' || c == 'u' || c == 'U'){
        return true;
    }
    else{
        return false;
    }
}

// Conta le consonanti in una stringa passata come parametro
int ContaConsonanti(char parola[]) {
    int cont = 0;
    for(int i=0; i < strlen(parola); i++) {
        if(!IsVocale(parola[i])) {
            cont++;
        }
    }
    return cont;
}

// Controlla che la stringa passata come parametro sia considerabile un nome italiano
bool ValidaNome(char nome[]){
    if(strlen(nome) < LEN_MIN_NOME){
        return false;
    }
    for(int i=0; i<strlen(nome); i++){
        if(!isalpha(nome[i]) && nome[i] != '\0') {
            return false;
        }
    }
    return true;
}

// Controlla che la stringa passata come parametro sia considerabile un cognome italiano
bool ValidaCognome(char cognome[]){
    if(strlen(cognome) < LEN_MIN_COGNOME){
        return false;
    }
    for(int i=0; i<strlen(cognome); i++){
        if(!isalpha(cognome[i]) && cognome[i] != '\0') {
            return false;
        }
    }
    return true;
}

// Controlla che la stringa passata come parametro sia considerabile un comune italiano
bool ValidaLuogoNascita(char luogoNascita[]){
    if(strlen(luogoNascita) < LEN_MIN_LUOGO_NASCITA){
        return false;
    }
    for(int i=0; i<strlen(luogoNascita); i++){
        if(!isalpha(luogoNascita[i]) && luogoNascita[i] != '\0') {
            return false;
        }
    }
    return true;
}

// Legge da console il nome del soggetto e lo scrive nella stringa passata come parametro
void LeggiNome(char stringa[]){
    bool controllo;
    do{
        printf("Inserisci il nome del soggetto: ");
        gets(stringa);
        controllo = ValidaNome(stringa);
        if(!controllo){
            printf("ERRORE. Inserisci un nome valido.\n");
        }
    }while(!controllo);
}

// Legge da console il cognome del soggetto e lo scrive nella stringa passata come parametro
void LeggiCognome(char stringa[]){
    bool controllo;
    do{
        printf("Inserisci il cognome del soggetto: ");
        gets(stringa);
        controllo = ValidaCognome(stringa);
        if(!controllo){
            printf("ERRORE. Inserisci un cognome valido.\n");
        }
    }while(!controllo);
}

// Restituisce la scelta dell'utente tra sesso maschile o femminile codificato come un carattere 'M' o 'F'
char LeggiSesso(){
    char sesso;
    do{
        printf("Inserisci il sesso del soggetto:\n"
               "\tM - Maschile\n"
               "\tF - Femminile\n"
               "Digitare la scelta: ");
        scanf("%c",&sesso);
        getchar();
        if(sesso != 'M' && sesso != 'F'){
            printf("ERRORE. Digita una scelta valida.");
        }
    }while(sesso != 'M' && sesso != 'F');
    return sesso;
}

// Restituisce la data di nascita inserita dall'utente e esegue contestualmente i controlli di validità
data LeggiDataNascita(){
    int anno, mese, giorno;
    int giorniMeseMax;
    bool bisestile;

    // Richiesta dell'anno
    do{
        // Ho scelto di stabilire un anno minimo in quanto assumo che non esistano persone vive nate prima del 1900
        printf("Inserisci l'anno di nascita (MIN: %d): ", ANNO_MIN);
        scanf("%d",&anno);
        if(anno < ANNO_MIN){
            printf("ERRORE. Inserisci un valore valido.\n");
        }
    }while(anno < ANNO_MIN);
    // Controllo se l'anno sia bisestile o non bisestile
    if(anno % 100 == 0){
        bisestile = anno % 400 == 0;
    }
    else{
        bisestile = anno % 4 == 0;
    }

    // Richiesta del mese
    do{
        printf("Inserisci il mese di nascita (Numeri da 1 a 12): ");
        scanf("%d",&mese);
        if(mese < 1 || mese > 12){
            printf("ERRORE. Inserisci un valore valido.\n");
        }
    }while(mese < 1 || mese > 12);

    // Richiesto il mese stabilisco il numero di giorni di quel mese in modo da poter validare l'input
    if(mese == 2){
        if(bisestile){
            giorniMeseMax = 29;
        }
        else{
            giorniMeseMax = 28;
        }
    }
    else if(mese == 4 || mese == 6 || mese == 9 || mese == 11){
        giorniMeseMax = 30;
    }
    else{
        giorniMeseMax = 31;
    }
    // Richiesta del giorno
    do{
        printf("Inserisci il giorno di nascita (Numeri da 1 a %d): ",giorniMeseMax);
        scanf("%d",&giorno);
        if(giorno < 1 || giorno > giorniMeseMax){
            printf("ERRORE. Inserisci un valore valido.\n");
        }
    }while(giorno < 1 || giorno > giorniMeseMax);

    data data = {giorno, mese, anno};
    return data;
}

// Legge da console il luogo di nascita del soggetto e lo scrive nella stringa passata come parametro
void LeggiLuogoNascita(char stringa[]){
    bool controllo;
    do{
        getchar();
        printf("Inserisci il luogo di nascita del soggetto: ");
        gets(stringa);
        controllo = ValidaLuogoNascita(stringa);
        if(!controllo){
            printf("ERRORE. Inserisci un luogo di nascita valido.\n");
        }
    }while(!controllo);
}

// Restituisce la codifica del nome della persona
char* CodificaNome(char nome[]) {
    // La funzione restituisce un puntatore ad una stringa allocata dinamicamente
    char *codNome = calloc(LEN_COD_NOME + 1, sizeof(char));
    // Controllo di avvenuta allocazione
    if(codNome == NULL){
        printf("ERRORE FATALE. Allocazione fallita.\n");
        exit(1);
    }
    // Uso una stringa temp per costruire la codifica per concatenazione
    char temp[2] = "\0\0";
    int numConsonanti = ContaConsonanti(nome), contConsonanti = 0;
    bool codiceIncompleto = false, lettereInsufficenti = false;

    strcpy(codNome, "");
    do {
        if(lettereInsufficenti){
            // Se le lettere sono insufficenti concateno il carattere di riempimento
            strcat(codNome, "X");
        }
        else {
            for (int i = 0; i < strlen(nome); i++) {
                if (codiceIncompleto) {
                    // Se il codice è incompleto aggiungo ad esso le vocali
                    if (isalpha(nome[i]) && IsVocale(nome[i])) {
                        temp[0] = nome[i];
                        strcat(codNome, temp);
                    }
                    if(strlen(codNome) == LEN_COD_NOME){
                        // Ad ogni vocale aggiunta controllo la lunghezza della codifica
                        break;
                    }
                } else {
                    // Inizio aggiungendo alla codifica le consonanti
                    if (isalpha(nome[i]) && !IsVocale(nome[i])) {
                        contConsonanti++;
                        if(numConsonanti >= 4) {
                            // Se il numero di consonanti è maggiore o ugale a 4 è necessario contarle
                            if (contConsonanti == 1 || contConsonanti == 3 || contConsonanti == 4) {
                                temp[0] = nome[i];
                                strcat(codNome, temp);
                            }
                        }
                        else {
                            // Altrimenti le preleviamo indipendentemente
                            temp[0] = nome[i];
                            strcat(codNome, temp);
                        }
                    }
                }
            }
        }
        // Se dalla stringa del nome non sono stato in grado di estrarre 3 caratterri modifico il valore di questa flag
        if (strlen(codNome) != LEN_COD_NOME) {
            if(codiceIncompleto){
                // Se la lunghezza non è coretta e il flag codiceIncompleto era già a valore TRUE
                // ho determinato di non avere lettere a sufficenza
                lettereInsufficenti = true;
            }
            codiceIncompleto = true;
        }
        else{
            codiceIncompleto = false;
        }
        // Ripeto la procedura per permettere l'aggiunta delle vocali e degli eventuali caratteri di riempimento
        // che durante la prima iterazione non vengono considerati
    } while (codiceIncompleto);

    // Converto tutta la stringa in maiuscolo
    for(int i=0; i<strlen(codNome); i++){
        codNome[i] = (char)toupper(codNome[i]);
    }

    return codNome;
}

// Restituisce la codifica del cognome della persona
char* CodificaCognome(char cognome[]) {
    // La funzione restituisce un puntatore ad una stringa allocata dinamicamente
    char *codCognome = calloc(LEN_COD_COGNOME + 1, sizeof(char));
    // Controllo di avvenuta allocazione
    if(codCognome == NULL){
        printf("ERRORE FATALE. Allocazione fallita.\n");
        exit(1);
    }
    // Uso una stringa temp per costruire la codifica per concatenazione
    char temp[2] = "\0\0";
    int contConsonanti = 0;
    bool codiceIncompleto = false, lettereInsufficenti = false;

    strcpy(codCognome, "");
    do {
        if(lettereInsufficenti){
            // Se le lettere sono insufficenti concateno il carattere di riempimento
            strcat(codCognome, "X");
        }
        else {
            for (int i = 0; i < strlen(cognome); i++) {
                if (codiceIncompleto) {
                    // Se il codice è incompleto aggiungo ad esso le vocali
                    if (isalpha(cognome[i]) && IsVocale(cognome[i])) {
                        temp[0] = cognome[i];
                        strcat(codCognome, temp);
                    }
                    if(strlen(codCognome) == LEN_COD_NOME){
                        // Ad ogni vocale aggiunta controllo la lunghezza della codifica
                        break;
                    }
                } else {
                    // Inizio aggiungendo alla codifica le consonanti
                    if (isalpha(cognome[i]) && !IsVocale(cognome[i]) && contConsonanti < 3) {
                        contConsonanti++;
                        temp[0] = cognome[i];
                        strcat(codCognome, temp);
                    }
                }
            }
        }
        // Se dalla stringa del cognome non sono stato in grado di estrarre 3 caratterri modifico il valore di questo flag
        if (strlen(codCognome) != LEN_COD_COGNOME) {
            if(codiceIncompleto){
                // Se la lunghezza non è coretta e il flag codiceIncompleto era già a valore TRUE
                // ho determinato di non avere lettere a sufficenza
                lettereInsufficenti = true;
            }
            codiceIncompleto = true;
        }
        else{
            codiceIncompleto = false;
        }
        // Ripeto la procedura per permettere l'aggiunta delle vocali e degli eventuali caratteri di riempimento
        // che durante la prima iterazione non vengono considerati
    } while (codiceIncompleto);

    // Converto tutta la stringa in maiuscolo
    for(int i=0; i<strlen(codCognome); i++){
        codCognome[i] = (char)toupper(codCognome[i]);
    }

    return codCognome;
}

// Restituisce la codifica della data di nascita della persona
char* CodificaDataNascita(data data, char sesso){
    // Utlizzo una stringa di appoggio per costruire la codifica per concatenazione
    char temp[5] = "\0\0\0\0\0";

    char *codificaData = calloc(LEN_COD_DN, sizeof(char));
    if(codificaData == NULL){
        printf("ERRORE FATALE. Allocazione fallita.\n");
        exit(1);
    }
    strcpy(codificaData, "");

    // Scrivo su una stringa di appoggio il contenuto dell'anno di nascita che è attualemte in formato "AAAA"
    snprintf(temp, sizeof(temp), "%d", data.anno);
    // Opero sui singoli caratteri per convertire il formato in "AA"
    temp[0] = temp[2];
    temp[1] = temp[3];
    temp[2] = '\0';
    temp[3] = '\0';
    strcat(codificaData, temp);
    // Rendo la stringa una stringa vuota
    temp[1] = '\0';
    temp[0] = '\0';

    // Recupero la lettera associata al mese dall'aposito alfabeto
    temp[0] = MESI[data.mese];
    strcat(codificaData, temp);
    strcpy(temp, "\0\0\0\0\0");

    // Modifico il valore del giorno di nascita in base al sesso
    if(sesso == 'F'){
        data.giorno += 40;
        // I giorni di nascita per i soggetti femminili figurano da 41 a 71 perciò sono tutti in formato "GG"
        snprintf(temp, sizeof(temp), "%d", data.giorno);
    }
    else{
        // I giorni di nascita per i soggetti maschili figurano invariati da 1 a 31 perciò ho la necessità di applicare
        // metodi diversi in base al valore del giorno in modo che tutti abbiano il formato "GG"
        if(data.giorno < 10){
            snprintf(temp, sizeof(temp), "%d%d", 0, data.giorno);
        }
        else{
            snprintf(temp, sizeof(temp), "%d", data.giorno);
        }
    }
    strcat(codificaData, temp);

    return codificaData;
}

// Restituisce il codice catastale del comune di nascita della persona
char* LeggiCodiceCatastale(char luogoNascita[]){
    // Il file codiciCatastali.csv contiene l'elenco di tutti i comuni italiani con i relativi codici catastali
    const char fileName[20] = "codiciCatastali.csv";
    FILE *codiciCatastali = fopen(fileName, "r");

    char stringaLetta[LEN_LUOGO_NASCITA] = "";
    bool carattereTrovato = false, luogoNascitaTrovato = false;
    int i, j;

    char* codCatastale = calloc(LEN_COD_CATASTALE+1, sizeof(char));
    if(codCatastale == NULL){
        printf("ERRORE FATALE. Allocazione fallita.\n");
        fclose(codiciCatastali);
        exit(1);
    }
    strcpy(codCatastale, "\0\0\0\0\0");

    // Leggo tutto il contenuto del file
    while(fgets(stringaLetta, LEN_LUOGO_NASCITA, codiciCatastali) != NULL){
        if(strstr(stringaLetta, luogoNascita) != 0) {
            // Se c'è una corrispondenza almeno parziale con il luogo di nascita
            luogoNascitaTrovato = true;
            i=0;
            while(i < strlen(stringaLetta) && stringaLetta[i] != DIV_CHAR){
                // Procedo a controllare la presenza di una corrispondenza esatta
                if(stringaLetta[i] != luogoNascita[i]){
                    luogoNascitaTrovato = false;
                    break;
                }
                i++;
            }
            // Avendo la necessita di compare una stringa nel formato "<luogo di nascita>" con una stringa nel formato
            // "<luogo di nascita>;<codice catastale>" non è possibile utilizzare la funzione strcmp()
            if(luogoNascitaTrovato) {
                // Se vi è una corrispondeza esatta provvedo ad estrare dalla stringa il codice catastale
                carattereTrovato = false;
                j = 0;
                for (i = 0; i < strlen(stringaLetta); i++) {
                    if (stringaLetta[i] == DIV_CHAR) {
                        carattereTrovato = true;
                        continue;
                    }
                    if (carattereTrovato && stringaLetta[i] != '\n') {
                        codCatastale[j] = stringaLetta[i];
                        j++;
                    }
                }
                // Completate le operazioni chiudo il file
                fclose(codiciCatastali);
                return codCatastale;
            }
        }
    }

    // Se tutte le iterazioni sul file dovessero essere completate significherebbe che il luogo di nascita specificato
    // non è presente nel file. A quel punto viene restituito un messaggio di errore.
    fclose(codiciCatastali);
    printf("ERRORE FATALE. Il luogo di nascita specificato non è presente nel nostro registro.\n");
    exit(2);
}

// Calcola il CIN partendo dal codice fiscale parziale
char CalcolaCIN(char codiceFiscaleParziale[]){
    int resto = 0;
    for(int i=0; i<strlen(codiceFiscaleParziale); i++){
        // Per ogni carattere che compone il codice fiscale parziale
        for(int j=0; j<strlen(CARATTERI); j++){
            // Cerco la sua posizione all'interno dello specifico alfabeto
            if(codiceFiscaleParziale[i] == CARATTERI[j]) {
                // Una volta trovato in base alla sua posizione incremento il valore della variabile resto
                // e ritorno al ciclo for superiore tramite l'istruzione break
                if ((i+1) % 2 == 0) {
                    resto += VALORE_CARATTERI_PARI[j];
                    break;
                }
                else {
                    resto += VALORE_CARATTERI_DISPARI[j];
                    break;
                }
            }
        }
    }
    // Calcolo il resto
    resto = resto % 26;
    // Restituisco il carattere corrispondendente del relativo alfabeto
    return CARATTERI_RESTO[resto];
}

// Restituisce il codice fiscale della persona
char* CalcolaCodiceFiscale(char codNome[], char codCognome[], char codDataNascita[], char codCatastale[]){
    char *codiceFiscale = calloc(LEN_CF + 1, sizeof(char));
    if(codiceFiscale == NULL){
        printf("ERRORE FATALE. Allocazione fallita.\n");
        exit(1);
    }
    strcpy(codiceFiscale, "");

    // Costruisco il codice fiscale per concatenazione delle varie parti.
    // Non sono necessari altri controlli in quanto sono svolti dalle singole funzioni di codifica delle singole parti
    strcat(codiceFiscale, codCognome);
    strcat(codiceFiscale, codNome);
    strcat(codiceFiscale, codDataNascita);
    strcat(codiceFiscale, codCatastale);
    codiceFiscale[strlen(codiceFiscale)] = CalcolaCIN(codiceFiscale);
    return codiceFiscale;
}

int main() {
    setlocale(LC_ALL, "it_IT");
    int scelta;

    // Variabili per leggere i dati del soggetto
    char nome[LEN_NOME];
    char cognome[LEN_COGNOME];
    char sesso;
    data dataNascita;
    char luogoNascita[LEN_LUOGO_NASCITA];

    // Messaggio di benvenuto
    printf("\n--- CALCOLATORE CODICE FISCALE ---\n"
           "Seguire la procedura di seguito descritta per calcolare il codice fiscale del soggetto.\n"
           "NOTA: È possibile calcolare il codice fiscale unicamente di soggetti nati in Italia.\n\n");

    // Lettura dei dati con controllo di inserimento
    do {
        // Lettura dei dati
        printf("Inserisci i dati del soggetto di cui desideri calcoalre il codice fiscale come di seguito richiesto:\n");
        LeggiNome(nome);
        LeggiCognome(cognome);
        sesso = LeggiSesso();
        dataNascita = LeggiDataNascita();
        LeggiLuogoNascita(luogoNascita);

        // Stampa riepilogativa
        printf("\n--- RIEPILOGO DEI DATI INSEIRITI ---\n"
               "\tNome: %s\n"
               "\tCognome: %s\n"
               "\tSesso: %c\n"
               "\tData di nascita: %d/%d/%d\n"
               "\tLuogo di nascita: %s\n",nome, cognome, sesso, dataNascita.giorno, dataNascita.mese, dataNascita.anno, luogoNascita);

        // Richiesta di verifica dei dati con validazione dell'input
        do{
            printf("I dati inseriti sono corretti?\n"
                   "\t1 - Si\n"
                   "\t0 - No\n"
                   "Digitare la scelta: ");
            scanf("%d",&scelta);
            if(scelta != 0 && scelta != 1){
                printf("ERRORE. Digita una scelta valida.");
            }
        }while(scelta != 0 && scelta != 1);
        getchar();
        // Se l'utente digita 0 chiedo nuovamente di inserire i dati
    }while(!scelta);

    // Calcolo delle singole codifiche
    char* codNome = CodificaNome(nome);
    char* codCognome = CodificaCognome(cognome);
    char* codDN = CodificaDataNascita(dataNascita, sesso);
    char* codCatastale = LeggiCodiceCatastale(luogoNascita);

    // Creazione del codice fiscale e stampa del risultato
    char* codiceFiscale = CalcolaCodiceFiscale(codNome, codCognome, codDN, codCatastale);
    printf("\nCodice fiscale generato corettamente:\n"
           "Codice fiscale: %s", codiceFiscale);

    // Libero la memoria allocata
    free(codNome);
    free(codCognome);
    free(codDN);
    free(codCatastale);
    free(codiceFiscale);

    return 0;
}
