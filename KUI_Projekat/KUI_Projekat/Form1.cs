using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;



namespace KUI_Projekat
{
    public partial class Form1 : Form
    {
        List<Korisnik> prijavljeni_korisnici = new List<Korisnik>();
        int brojac = 1;
        long ocitanje_long = 0;
        string uspjesnost = "";
        string pocetno_vrijeme = "";
        string vrijeme_kreiranja = "";
        
        public Form1()
        {
            InitializeComponent();
            CheckForIllegalCrossThreadCalls = false;
            stlbDateTime.Text = DateTime.Now.ToString();
            try
            {
                serialPort1.Open();
            }
            catch
            {
                MessageBox.Show("Nije otvoren port!");
            }
        }

        private void SerialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            Korisnik korisnik = null;
            string ocitanje = serialPort1.ReadLine();
            string dostupno = "";
            try
            {
                if (ocitanje.Length > 8)
                {
                    int duzina = ocitanje.Length;
                    ocitanje_long = long.Parse(ocitanje.Substring(duzina - 8));
                }
                else
                {
                    ocitanje_long = long.Parse(ocitanje);
                }
                
            }
            catch
            {
                dostupno =ocitanje+"/"+ DateTime.Now.ToString();
                uspjesnost = ocitanje;
                //MessageBox.Show("Nije uspjesno ocitan RFID");
            }
            if (dostupno !="")
            {
                int nadjen = 0;
                korisnik = new Korisnik(ocitanje_long, uspjesnost);
                foreach (Korisnik kor in prijavljeni_korisnici)
                {
                    if (kor.Rfid_korisnika == korisnik.Rfid_korisnika)
                        nadjen = 1;
                }
                if (nadjen==0)
                {
                    prijavljeni_korisnici.Add(new Korisnik(ocitanje_long, uspjesnost));
                    lbPristupi.Items.Add(brojac + ". " + prijavljeni_korisnici[prijavljeni_korisnici.Count - 1].ToString());
                    brojac++;
                }
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            serialPort1.WriteLine("Da");
            pocetno_vrijeme = DateTime.Now.ToString();
        }

        private void TimerVrijeme_Tick(object sender, EventArgs e)
        {
            stlbDateTime.Text = DateTime.Now.ToString();
        }



        private void AboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AboutBox1 about = new AboutBox1();
            about.ShowDialog();
        }

        private void ExitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void LbPristupi_Click(object sender, EventArgs e)
        {
            Korisnik kliknuti_korisnik = null;
            int index_kliknutog;
            if (lbPristupi.SelectedItem != null)
            {
                string temp = lbPristupi.SelectedItem.ToString();
                index_kliknutog = lbPristupi.SelectedIndex;
                string temp2= "";
                foreach (Korisnik korisnik in prijavljeni_korisnici)
                {
                    if (temp.Contains(korisnik.ToString()))
                    {
                        kliknuti_korisnik = korisnik;
                        temp2 ="Korisnik "+ korisnik.Rfid_korisnika+"";
                        switch (korisnik.Odobren_korisnik)
                        {
                            case "Postoji u EEPROM-u!\r":
                                temp2 += " je unesen u EEPROM. Da li želite da uklonite korisnika iz EEPROM-a?";
                                break;
                            case "Ne postoji u EEPROM-u!\r":
                                temp2 += " nije unesen u EEPROM.Da li želite da dodate korisnika u EEPROM?";
                                break;
                            default:
                                break;     
                        }
                    }
                }
                DialogResult dr=MessageBox.Show(temp2, "RFID", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (dr == DialogResult.Yes)
                {
                    serialPort1.WriteLine(kliknuti_korisnik.Rfid_korisnika+"");
                    switch (kliknuti_korisnik.Odobren_korisnik)
                    {
                        case "Postoji u EEPROM-u!\r":
                            foreach (Korisnik korisnik in prijavljeni_korisnici)
                            {
                                if (korisnik.Rfid_korisnika == kliknuti_korisnik.Rfid_korisnika)
                                {
                                    korisnik.Odobren_korisnik = "Ne postoji u EEPROM-u!\r";
                                    lbPristupi.Items[index_kliknutog] =(index_kliknutog+1)+". "+ korisnik.ToString();
                                }
                            }
                            break;
                        case "Ne postoji u EEPROM-u!\r":
                            foreach (Korisnik korisnik in prijavljeni_korisnici)
                            {
                                if (korisnik.Rfid_korisnika == kliknuti_korisnik.Rfid_korisnika)
                                {
                                    korisnik.Odobren_korisnik = "Postoji u EEPROM-u!\r";
                                    lbPristupi.Items[index_kliknutog] =(index_kliknutog+1)+". "+ korisnik.ToString();
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            serialPort1.WriteLine("Ne");
            serialPort1.Close();
        }

        void GenerisiTekstualni()
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter= "Text document (*.txt)|*.txt";
            DialogResult dr = sfd.ShowDialog();
            if (dr == DialogResult.OK)
            {
                vrijeme_kreiranja = DateTime.Now.ToString();
                string ispis="Tagovanja u periodu od "+pocetno_vrijeme+" do "+ vrijeme_kreiranja+"\n";
                foreach (object logovanje in lbPristupi.Items)
                {
                    ispis += logovanje;
                }
                File.WriteAllText(sfd.FileName, ispis);
                MessageBox.Show("Uspješno ste kreirali fajl!", "Kreiranje tekstualnog fajla", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void BtnGenerisiTekstualni_Click(object sender, EventArgs e)
        {
            if (lbPristupi.Items.Count > 0)
            { GenerisiTekstualni(); }
            else
            {
                MessageBox.Show("Ne postoje pokušaji prisustva od trenutka paljenja aplikacije!");
            }

        }
    }
}
