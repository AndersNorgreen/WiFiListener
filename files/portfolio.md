# Portfolio (Søren, Affe, Michael og Anders)

## Hvordan ser løsningen ud
- [ ] Arkitektur
- [ ] Teknologier

![WifiListener Diagram](./diagrams/WiFiSnifferDiagram_export.png)

![WifiListener Flowchart](./diagrams/WiFiSnifferFlowchart_export.png)

## Dataprocess
- [ ] primær dataopsamling
- [ ] data berigelse
- [ ] beregning
- [ ] data lagring
- [ ] evt data visualisering

## Datasikkerhed

### Følsomhed
  - [ ] hvad er personlige data?
  Personoplysninger er sikret gennem Databeskyttelsesfordordningen GDPR. Personoplysninger kan omhandle enten __identificerede__ eller __identificerbare__ personer. Der skelnes ligeledes mellem almindelige personoplysninger og følsomme personoplysninger.

  Almindelige oplysninger er navn, mail, telefonummer, adresse mm., altså oplysninger, der som oftest indgår i f.eks. nethandel. Der er således ikke de samme begrænsninger på brugen og opbevaringen af oplysninger anerkendt som almindelige.

  Følsomme oplysninger omhandler race, religion, politisk overbevisning, helbredsoplysninger, juridiske oplysninger og lignende. Der kræves udtrykkeligt samtykke fra en person, for lovligt at behandle denne data.
  
  En nærmere gennemgang kan findes [her](https://gdpr.dk/persondataforordningen/hvad-er-personoplysninger/).


  - [ ] hvad er risikoen ved at behandle andre folks personlige data?
  
  - [ ] i hvor høj grad er de data projektet benytter personlige?
  
  Det kan diskuteres, i hvor høj grad det data, som benyttes i nærværende projekt, er beskyttet under GDPR. Der er hovedsageligt tale om MAC-adresser og positionsdata. 
  
  MAC-adresser er unikke til hver enhed, og det er derfor i teorien muligt at identificere enhedens ejer ud fra adressen. 
  
  Dog gemmes MAC-adresser allerede sammen med enhedens IP-adresse på alle netværksenheder i den såkaldte arp-tabel til brug for kommunikation over netværket. Der kan derfor argumenteres for, at informationen allerede er tilgængelig, hvis det ønskes. Samtidig er de nemme at skifte til trods for deres forbindelse til enheden - mange moderne enheder ændrer faktisk deres MAC-adresse automatisk, hver gang de forbinder til et netværk.
  
  MAC-adresser ligger således i et gråzonefelt, når det kommer til persondataoplysninger.

  Enhedes position, eller lokaliseringsdata, falder under _identificerbare_ personoplysninger. Når der i projektet trianguleres en position for en bestemt enhed, vil der derfor være tale om data beskyttet under GDPR. Da positionsdata anvendes af mange forskellige leverandører, ligger det i kategorien almindelige personoplysninger.

  Projektet tager højde for udfordringerne ved behandling af data gennem anonymisering og kryptering, samt ved at slette data så snart, det kan lade sig gøre. 

  - [ ] er data mere personlige når de beriges?

  Dette kan variere alt efter oplysningernes karakter, men overordnet set kan man sige, at jo flere oplysninger, der kan knyttes til data, jo nærmere kan man komme på at identificere en person.

  - [ ] hvor længe er der brug for at have personlige data i projektet?

  Projektet har behov for de mulige identificerbare oplysninger i meget kort tid. Så snart en opsamlet datablok er afsendt til den centrale MQTT-server, glemmes alle oplysninger om MAC-adresser. Det afsendte data vil være anonymiseret.

### Beskyttelse
  - [ ] hvem kan få adgang?
  - [ ] hvor svært er det for udenforstående at skaffe sig adgang til data?
  - [ ] eller at lytte med?
