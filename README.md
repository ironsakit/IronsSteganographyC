# PNG Steganography in C

Un tool leggero e a basso livello scritto in C per nascondere messaggi di testo segreti all'interno di immagini PNG, senza alterare in alcun modo l'aspetto visivo dell'immagine originale.

**Autore:** Ferro Mattia

## 1. Descrizione
Questo progetto dimostra come manipolare direttamente i file binari e la struttura a blocchi (chunk) del formato PNG. Il programma legge un'immagine valida, inietta un chunk personalizzato contenente il messaggio segreto e ricalcola i codici di controllo, generando un "clone" infetto ma perfettamente leggibile dai normali visualizzatori di immagini.

## Funzionalità
* **Iniezione (Steganografia):** Nasconde una stringa di testo dentro un nuovo file `secret.png`.
* **Estrazione:** Scansiona un'immagine PNG alla ricerca del chunk segreto e ne estrae il testo in chiaro.
* **Integrità dei Dati:** Calcolo automatico del CRC (Cyclic Redundancy Check) per garantire che il file PNG rimanga valido e non risulti corrotto.
* **Gestione Endianness:** Compatibilità di rete garantita tramite la conversione Big-Endian / Little-Endian (`htonl`, `ntohl`).

## 2. Come funziona sotto il cofano
Il formato PNG è composto da vari blocchi (chunk) come `IHDR` (header), `IDAT` (dati immagine) e `IEND` (fine). 
Questo programma sfrutta la flessibilità del formato per creare un chunk non standard chiamato **`prIv`**. 
1. I visualizzatori di immagini normali non riconoscono il blocco `prIv` e lo ignorano, mostrando l'immagine normalmente.
2. L'estrattore, invece, "salta" i blocchi standard calcolando dinamicamente i byte da ignorare tramite la funzione `fseek`, fino a scovare e decodificare il blocco `prIv`.

## 3. Prerequisiti e Compilazione
Il progetto è stato sviluppato in ambiente Windows. Utilizza la libreria di sistema `winsock2.h` per la gestione dell'endianness.
<br>`TODO`: renderlo portatile.

### 4. Compilazione manuale (GCC/MinGW)
Per compilare il programma da riga di comando, assicurati di linkare la libreria `ws2_32`:
```bash
gcc main.c PNG.c -o steganography.exe -lws2_32
