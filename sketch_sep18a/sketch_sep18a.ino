//Uključivanje potrebnih biblioteka
#include <EEPROM.h>
#include <SoftwareSerial.h>


#define RED 10
#define BLUE 11
#define GREEN 9
#define RELAY 7
#define TX 6
#define RX 8

int brojac=1;
byte broj_tagova;
long procitan_tag=0;
String temp="";//pomoćna promjenljiva koja će ili obavještavati o tome da li je naša aplikacija uključena ili o RFID-u koji uklanjamo sa ili dodajemo na EEPROM 
long poslati_tag_konvertovan=0;// pošto je sve što se čita sa Serial-a u formi string-a, a šifra RFID-a je isključivo brojni podatak, poželjna je konverzija iz String u long
String app_otvorena="Ne";
int redni_broj_taga=0;
unsigned extract_tag();
const int BUFFER_SIZE = 14; // skenirani RFID nosi sa sobom vrijednost od 14 B
const int DATA_SIZE = 10; // 10 B DATA (2 B verzija + 8 B šifra)
const int DATA_VERSION_SIZE = 2; // Verzija 2 B
const int DATA_TAG_SIZE = 8; // Šifra 2 B
const int CHECKSUM_SIZE = 2; // Checksum 2 B
SoftwareSerial ssrfid = SoftwareSerial(TX,RX);
uint8_t buffer[BUFFER_SIZE]; // niz koji će čuvati vrijednosti skeniranog taga
int buffer_index = 0; // početni index niza je 0
long hexstr_to_value(char *str, unsigned int length);
void EEPROMWritelong(int redniBroj, long value);
long EEPROMReadLong(int rb);
void obrisiTag(long poslati);
void dodajTag(long poslati);

void setup() {
 Serial.begin(9600); 
 ssrfid.begin(9600);
 ssrfid.listen(); 
 pinMode(RED,OUTPUT); //definiše se pin RED(10) kao izlazni
 pinMode(GREEN,OUTPUT);//definiše se pin GREEN(9) kao izlazni
 pinMode(BLUE,OUTPUT);//definiše se pin BLUE(11) kao izlazni
 pinMode(RELAY,OUTPUT);//definiše se pin RELAY(7) kao izlazni
 broj_tagova=EEPROM.read(0);//na prvoj poziciji u EEPROM-u nalazi se broj RFID-ova koji imaju pravo na pristup
}
void loop() {
  if(Serial.available())
  {
    temp=Serial.readString();
    if(isDigit(temp[0]))
    {
      poslati_tag_konvertovan=temp.toInt();
      int nadjen=0;
      for(int i=0;i<broj_tagova;i++)
        {
          long tekuci_tag=EEPROMReadLong(i);
          if(poslati_tag_konvertovan==tekuci_tag)
          {
              nadjen=1;
              redni_broj_taga=i;
              break;
          }
        }
      if(nadjen==0)
      {
        //dodajem tagove
        dodajTag(poslati_tag_konvertovan);
      }
      else
      {
        //brisem tagove
         obrisiTag(poslati_tag_konvertovan);
      }
    }
      else if(isAlpha(temp[0]))
      {
        app_otvorena=temp;
      }
  }
  if (ssrfid.available() > 0){
    bool call_extract_tag = false;
    int ssvalue = ssrfid.read(); // read 
    if (ssvalue == -1) { // no data was read, -1 je prazan string
      return;
    }
    if (ssvalue == 2) { // RDM630/RDM6300 found a tag => tag incoming, HEAD je uvijek 2
      buffer_index = 0;
    } else if (ssvalue == 3) { // tag has been fully transmitted, TAIL je uvijek 3
      call_extract_tag = true; // extract tag at the end of the function call
    }
    if (buffer_index >= BUFFER_SIZE) { // checking for a buffer overflow (It's very unlikely that an buffer overflow comes up!)
      Serial.println("Prekoracenje!");
      return;
    }
    
    buffer[buffer_index++] = ssvalue; // everything is alright => copy current value to buffer
    if (call_extract_tag == true) {
      if (buffer_index == BUFFER_SIZE) {
        unsigned tag = extract_tag();
        digitalWrite(BLUE,HIGH);
        delay(1000);
        digitalWrite(BLUE,LOW);
        int nadjen=0;
        for(int i=0;i<broj_tagova;i++)
        {
          long tekuci_tag=EEPROMReadLong(i);
          if(procitan_tag==tekuci_tag)
          {
              digitalWrite(GREEN,HIGH);
              if(app_otvorena[0]=='N'){
                digitalWrite(RELAY,HIGH);
              }
              Serial.println("Postoji u EEPROM-u!");
              nadjen=1;
              break;
          }
        }
        if(nadjen==0)
          {
            digitalWrite(RED,HIGH);
            Serial.println("Ne postoji u EEPROM-u!");
          }
        delay(1000);
        digitalWrite(RELAY,LOW);
        digitalWrite(GREEN,LOW);
        digitalWrite(RED,LOW);
        ssrfid.end();
        ssrfid.begin(9600);
      } else { // došlo je do greške, očitanje pokrenuti od početka
        buffer_index = 0;
        return;
      }
    }    
  } 
}

//Funkcija koja prikuplja HEX vrijednosti sa RFID-a
unsigned extract_tag() {
    uint8_t msg_head = buffer[0];
    uint8_t *msg_data = buffer + 1; //kupi sve vrijednosti od buffer[1] pa nadalje
    uint8_t *msg_data_version = msg_data;//kupi sve vrijednosti od msg_data[0] pa nadalje do kraja niza
    uint8_t *msg_data_tag = msg_data + 2;//kupi sve vrijednosti od msg_data[2] pa nadalje
    uint8_t *msg_checksum = buffer + 11; // kupi od buffer[11] pa nadalje
    uint8_t msg_tail = buffer[13];
    long tag = hexstr_to_value(msg_data_tag, DATA_TAG_SIZE);
    procitan_tag=tag;
    Serial.println(tag);
    return tag;
}

//Funkcija koja vrši konverziju HEX vrijednosti u dekadne
long hexstr_to_value(char *str, unsigned int length) {
  char* copy = malloc((sizeof(char) * length) + 1); //dinamičko alociranje memorije 
  memcpy(copy, str, sizeof(char) * length);// kopiranje sadržaja memorije s jedne lokacije na drugu
  copy[length] = '\0'; //string se mora završiti karakterom '\0'
  //kopira sadrzaj iz str u copy i na kraj postavlja NULL karakter pokazatelj kraja stringa
  long value = strtol(copy, NULL, 16);  //konvertovanje stringa u long
  free(copy); //oslobadjanje zauzete memorije
  return value;
}

//Funkcija koja vrši upisivanje 4B fajlova u EEPROM
void EEPROMWritelong(int redniBroj, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8 ) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
 
  EEPROM.write(1 + redniBroj *4, four);
  EEPROM.write(1 + redniBroj *4 + 1, three);
  EEPROM.write(1 + redniBroj *4 + 2, two);
  EEPROM.write(1 + redniBroj *4 + 3, one);
}

//Funkcija koja vrši čitanje 4B fajlova iz EEPROM-a
long EEPROMReadLong(int rb) {
  long four = EEPROM.read(1 + rb*4);
  long three = EEPROM.read(1 + rb*4 + 1);
  long two = EEPROM.read(1 + rb*4 + 2);
  long one = EEPROM.read(1 + rb*4 + 3);
  return ((four << 0) & 0xFF) + ((three << 8 ) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

//Funkcija koja vrši brisanje RFID-a iz niza RFID-ova sadržanih u EEPROM-u
void obrisiTag(long poslati){
byte brojRegistrovanih = EEPROM.read(0);
int pozicija=-1;
for(int i =0; i< brojRegistrovanih; i++)
{
  long trenutni = EEPROMReadLong(i);
  if(trenutni == poslati){
    pozicija = i;
    break;
  }
}
if(pozicija!=-1)
  {
  for(int i = pozicija; i< brojRegistrovanih-1; i++)
    {
     EEPROMWritelong(i, EEPROMReadLong(i+1));
    }
EEPROM.write(0, brojRegistrovanih-1);
broj_tagova=brojRegistrovanih-1;
  }

}

//Funkcija koja vrši dodavanje RFID-a na kraj niza RFID-ova sadržanih u EEPROM-u
void dodajTag(long poslati)
{
  byte brojRegistrovanih=EEPROM.read(0);
  EEPROMWritelong(brojRegistrovanih,poslati);
  brojRegistrovanih++;
  broj_tagova=brojRegistrovanih;
  EEPROM.write(0,brojRegistrovanih);
}
