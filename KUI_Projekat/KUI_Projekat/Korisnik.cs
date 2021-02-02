using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KUI_Projekat
{
    class Korisnik
    {
        
        private long rfid_korisnika;
        private string odobren_korisnik;

        
        public long Rfid_korisnika { get => rfid_korisnika; set => rfid_korisnika = value; }
        public string Odobren_korisnik { get => odobren_korisnik; set => odobren_korisnik = value; }

        public Korisnik(long rfid_korisnika,string odobren)
        {
            
            Rfid_korisnika = rfid_korisnika;
            Odobren_korisnik = odobren;
        }

        public override string ToString()
        {
            return Rfid_korisnika+"/"+Odobren_korisnik;
        }

    }
}
