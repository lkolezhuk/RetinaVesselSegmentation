--------------------------------------------------------------------
Generazione e compilazione di un progetto AIA che utilizza OpenCV
--------------------------------------------------------------------
N.B.: In questa guida viene usato Visual Studio 2012, ma può essere
sostituito con qualsiasi altro ambiente di sviluppo/compilazione sup-
portato da CMake (ad es. Eclipse, XCode, Unix Makefile, ecc.) anche
su piattaforme diverse da Windows (MacOS, Linux).
In tali casi, i requisiti e le istruzioni per Windows vanno opportu-
namente adattate alla piattaforma hardware/software utilizzata.
--------------------------------------------------------------------
Requisiti:

-  Architettura x64
-  S.O. Windows 7 o più recente
-  Controllo dell'account utente disattivato (cercare su Google come)
-  Antivirus ridotto alle funzionalità minime (meglio se disattivato)
-  CMake installato (https://cmake.org/)
-  OpenCV installato (versioni 2.*, NO versione 3!)
-  Visual Studio 2012 installato ed avviato almeno una volta
-  Progetto di esempio fornito dal docente
--------------------------------------------------------------------
Procedura:

1. Avviare CMake.

2. In corrispondenza di "Where is the source code", inserire la car-
   tella contenente i file sorgenti.
   *** ATTENZIONE ***: evitare cartelle il cui percorso contiene ca-
   ratteri accentati e/o spazi.
   
3. In corrispondenza di  "Where to build the binaries", inserire  la
   cartella  dove verranno generati i  file di progetto. Questa deve
   essere necessariamente diversa dalla cartella che contiene i sor-
   genti.
   *** ATTENZIONE ***: evitare cartelle il cui percorso contiene ca-
   ratteri accentati e/o spazi.

4. Premere il tasto "Configure" e selezionare  dal menù a tendina il
   generatore "Visual Studio 11 2012 Win64" e premere "Finish".
   
5. CMake effettuerà un tentativo  di compilazione di codice C++  con
   il compilatore indicato al passo 4 e controllerà che tutte le di-
   pendenze esterna (inclusa quella da OpenCV) siano soddisfatte.
   Se viene mostrato un errore relativo al compilatore Visual Studio
   significa che uno o più requisiti non sono soddisfatti.  
   Se viene mostrato solo un errore relativo ad OpenCV, va tutto ok.

6. In corrispondenza di "OpenCV_DIR", bisogna inserire il path della
   cartella “build” di OpenCV. 

7. Premere di  nuovo “Configure” e, se stavolta non ci  sono errori,
   proseguire al passo successivo, altrimenti rivedere i requisiti e
   ripetere i passi 1-7.
   
8. Premere su “Generate” per generare i file di progetto che appari-
   ranno nella cartella indicata al passo 3.

9. Da questo momento in poi, i sorgenti contenuti nella cartella in-
   dicata al passo 2 ed il progetto generato al passo 8  sono indis-
   solubilmente legati tra loro. Lo spostamento e/o la rinominazione
   di tali file e cartelle impedirà il corretto flusso di gestione e
   compilazione del progetto.

10.Prima ancora di aprire con Visual Studio il progetto appena gene-
   rato, bisogna aggiungere le .dll di OpenCV al path di  sistema in 
   modo tale che l’eseguibile del nostro progetto  possa “vedere” le 
   dll di OpenCV  e quindi  collegarle a runtime. Per farlo, bisogna 
   andare su Pannello di Controllo  > Sistema >  Impostazioni di si-
   stema avanzate  > Variabili d’ambiente  > Variabili di  sistema > 
   selezionare  “Path” e cliccare  su “Modifica” e  alla  fine della
   stringa inserire il carattere di separazione “;” seguito dal path
   della cartella dove si trovano le .dll OpenCV per l’architettura
   e compilatore utilizzati > cliccare su OK a cascata. 
   Ad esempio, io ho inserito ";C:\Program Files\OpenCV-2.4.4\opencv
   \build\x64\vc11\bin” in quanto la mia architettura è x64  ed uti-
   lizzo vc11 (compilatore C++ di Visual Studio 2012). 
   *** ATTENZIONE ***: se si cancella per sbaglio il contenuto della
   variabile di sistema "Path", alcuni dei programmi installati sul-
   la macchina cesseranno di funzionare.

11.Aprire,  con Visual Studio,  il file .sln (solution)  generato al
   passo 8.

12.Selezionare la modalità “Release” al posto di “Debug” in alto  (a 
   meno che non si voglia  fare il debugging,  che consente l'esecu-
   zione  linea-per-linea ma rende  il compilato circa 100 volte più
   lento).

13.Cliccare col tasto dx sul progetto “project0” nella lista  di sini-
   stra e selezionare “Set as startup project”;

14.Compilare ed eseguire con Ctrl + F5 (equivalente a Build  > Start
   without debugging). 
   Se si verifica un errore di DLL mancanti,  ripetere accuratamente 
   il passo 10 e riavviare Visual Studio.
   Se l'eseguibile non viene avviato, probabilmente la causa è l'an-
   tivirus o il controllo account utente (rivedere i requisiti).
   Se si verificano altri tipi di errori, questi  sono probabilmente
   dovuti a bug del codice, la cui risoluzione  esula dalla presente
   guida.
--------------------------------------------------------------------