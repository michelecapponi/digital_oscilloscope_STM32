from threading import Thread
from serial import Serial
from time import sleep


def check_for_valid_connection(device_name="", baud_rate=0, ):
    while True:
        try:
            ser = Serial(device_name, baud_rate)

        except:
            print("Waiting for valid connection")
            pass
        else:
            return ser


class srl_data_rec_loop(Thread):

    def __init__(self, ser):  # do la possibilità all'utente di inizializzare la lista di
        super().__init__()  # inizializzo l'ambiente della classe padre Thread
        self.ser = ser
        self.data_buffer = []  # inizializzo il buffer per i dati
        self.ch1_data = []  # definisco una lista accessibile dall'esterno che deve contenere i 512 campioni per la forma d'onda1
        self.ch2_data = []  # definisco una lista accessibile dall'esterno che deve contenere i 512 campioni per la forma d'onda2
        self.buffer_size = 2048  # dimensione del buffer esadecimale che deve contenere
        self.channel_size = int(self.buffer_size / 4)  # dimensione dei canali ch1 e ch2
        self.encoding = "ASCII"
        self.rc_flag = 0
        self.stop_flag = 0

    def check_start(self):
        while (True):
            self.serial_data = self.ser.read(1).decode(
                self.encoding)  # mi aspetto di ricevere un carattere * secondo la codifica ascii allora decodifico il byte ricevuto secondo la codifica ASCII

            if (self.serial_data == "*"):
                return  # se ho ricevuto lo start bit posso cominciare la ricezione dei byte di dato altrimenti qualsiasi altro carattere lo ignoro anche se erano byte di una sequenza precedente
            else:
                print("attesa start byte")
                pass

    def receiving_data(self):
        self.serial_data = self.ser.read(1).decode(self.encoding)

    def check_for_valid_data(self):  # controllo se il carattere ricevuto è hex oppoure se un #
        try:
            data = int(self.serial_data, 16)  # verifico se il carattere ricevuto è un hex
        except ValueError:
            if (self.serial_data == "#"):
                return 2  # ritorno 2 se il cattere ricevuto è il terminatore di sequenza
            else:

                print("Non valid data received")
                return -1  # ritorno -1 se non è ne il terminatore e nemmeno lo stop byte
        else:  # viene eseguito solo se il try va a buon fine
            return 1  # ritorno 1 se il carattere ricevuto è hex

    def charge_data_buffer(self):  # funzione che carica il buffer di ricezione
        self.data_buffer.append(self.serial_data)  # carico il dato ricevuto nel buffer

    def split_and_override_channels(self):
        self.ch1_data = []
        self.ch2_data = []
        j = 0
        for i in range(0, self.buffer_size, 2):  # scansiono la lista dei 2048 caratteri hex a step di due
            tmp = "".join(
                self.data_buffer[i:(i + 2)])  # creo una stringa temporanea contentente due caratteri hex consecutivi
            if j % 2 == 0:  # inserisco il dato alternativamente nel canale 1 e nel canale 2
                self.ch1_data.append(int(tmp, 16))
            else:
                self.ch2_data.append(int(tmp, 16))
            j += 1  # incremento variabile di controllo canale

    def clear_data_buffer(self):
        self.data_buffer = []

    def run(self):
        while not self.stop_flag:  # ogni volta che ricevo correttamente *hexhexhexhex...# riparto da qui

            self.check_start()  # rimango in attesa dello start byte, finchè non lo ricevo non devo fare nulla (ignoro tutto quello che arriva se non mi è arrivato lo start)
            print("ricevuto start byte")
            while (True):
                if (len(self.data_buffer) < self.buffer_size):
                    self.receiving_data()  # effettuo la lettura di un byte solo se il buffer non è pieno
                    res = self.check_for_valid_data()
                    if (res == 2):  # se entro in questo if ho ricevuto lo stop bit prima di aver riempito il buffer
                        print("Error: Stop byte non previsto")
                        self.clear_data_buffer()  # se ho ricevuto anticipatamente lo stop byte svuoto il buffer e torno in attesa dello start byte
                        break  # se ho ricevuto lo stop byte in maniera inaspettata ritorno in attesa dello start byte
                    elif (res == -1):  # ricevuto carattere non atteso
                        print("Error: Dato ricevuto non valido")
                        self.clear_data_buffer()  # se ho ricevuto un carattere non atteso svuoto il buffer e torno in attesa dello start byte
                        break  # se ho ricevuto un carattere non atteso torno in attesa dello start byte
                    elif (res == 1):
                        self.charge_data_buffer()
                if (len(self.data_buffer) == self.buffer_size):
                    self.receiving_data()  # se il buffer è pieno mi aspetto di ricevere lo stop byte
                    res = self.check_for_valid_data()  # verifico il dato ricevuto
                    if (res == 2):  # verifico se il dato ricevuto è un #
                        print("ricevuto stop byte")
                        self.split_and_override_channels()  # se ho ricevuto lo stop byte vado ad aggiornare i canali con i nuovi dati
                        self.rc_flag = 1  # imposto un flag di fine ricezione per avvisare il main program che dovrà aggiornare il buffer
                        self.clear_data_buffer()  # effettuo il reset del buffer
                        print(self.ch1_data)
                        print(self.ch2_data)
                        break
                    if (res != 2):
                        print("Error: invalid stop byte received")
                        self.clear_data_buffer()  # se ho ricevuto un byte di stop errato butto via tutto e torno al loop più esterno in attesa dello start byte
                        break


class srl_data_write_loop(Thread):
    default_samling_period = 1000000
    default_trigger_level = 128
    default_trigger_type = 0

    def __init__(self, ser):
        super().__init__()  # è come chiamare l'init di thread sull'oggetto self
        self.ser = ser
        self.sampling_period = 1000000  # imposto un parametro di default della classe che è il periodo di campionamento di default
        self.trigger_level = 128  # imposto un parametro di default della classe che è il livello di trigger
        self.trigger_type = 0  # imposto un parametro di default della classe per il trigger type
        self.commands_header = {'trigger_type': "TT", "trigger_level": 'TL',
                                "sampling_period": 'SP'}  # imposto un dizionario di default della classe per le intestazioni dei comandi
        self.start_byte = "*"  # imposto un parametro di default della classe per lo start
        self.stop_byte= "#"  # imposto un parametro di default della classe per lo stop
        self.encoding = "ASCII"
        self.stop_flag = 0

    def init_config(
            self):  # definisco una funzione che invia la configurazione di default all'oscilloscopio può essere invocata in maniera asincrona per resettare l'oscilloscopio
        self.send_sampling_period(
            reset_osc=True)  # invio il comando all'oscilloscopio per il setting del periodo di campionamento di default
        self.send_trigger_type(
            reset_osc=True)  # invio il comando all'oscilloscopio per il setting del trigger type di default
        self.send_trigger_level(
            reset_osc=True)  # invio il comando all'oscilloscopio per il setting del trigger level di default

    def send_sampling_period(self, reset_osc=False):
        if reset_osc == False:  # è stato richiesto un invio di una comando ma non è il comando di reset
            sampling_period = self.sampling_period
        elif reset_osc == True:  # è stato richiesto un invio di un comando ed è il reset
            sampling_period = self.default_samling_period
        self.ser.write(self.start_byte.encode(self.encoding))  # invio lo star byte
        hex_sampling_period = "%08x" % sampling_period  # converto in hex su due digit il sampling period
        self.ser.write(self.commands_header['sampling_period'].encode(self.encoding))
        self.ser.write(hex_sampling_period.encode(self.encoding))  # invio il sampling period convertito in hex
        self.ser.write(self.stop_byte.encode(self.encoding))


    def send_trigger_level(self, reset_osc=False):  # imposto come predefinito l'invio della configurazione non di reset
        if reset_osc == False:
            trigger_level = self.trigger_level  # trigger level assume il valore della variabile trigger level se non sto facendo un reset del dispositivo
        elif reset_osc == True:
            trigger_level = self.default_trigger_level  # trigger level assume il valore della variabile trigger level se sto facendo un reset
        self.ser.write(self.start_byte.encode(self.encoding))  # invio lo star byte
        self.ser.write(self.commands_header['trigger_level'].encode(self.encoding))  # invio l'header del comando di trigger level
        hex_trigger_level = "%02x" % trigger_level  # converto in hex su due digit il trigger level
        self.ser.write(hex_trigger_level.encode(self.encoding))  # invio il trigger level convertito in hex
        self.ser.write(self.stop_byte.encode(self.encoding))


    def send_trigger_type(self, reset_osc=False):
        if reset_osc == False:
            trigger_type = self.trigger_type
        elif reset_osc == True:
            trigger_type = self.default_trigger_type
        self.ser.write(self.start_byte.encode(self.encoding))  # invio lo star byte
        self.ser.write(self.commands_header['trigger_type'].encode(self.encoding))  # invio l'header del comando di trigger type
        hex_trigger_level = "%02x" % int(trigger_type)  # converto in hex su due digit il trigger type #modificato (casting ad intero forzato)
        self.ser.write(hex_trigger_level.encode(self.encoding))  # invio il trigger type convertito in hex
        self.ser.write(self.stop_byte.encode(self.encoding))

    def run(self):

        temp_sampling_period = self.sampling_period
        temp_trigger_level = self.trigger_level
        temp_trigger_type = self.trigger_type

        while not self.stop_flag:
            if (
                    temp_sampling_period != self.sampling_period):  # controllo se l'utente ha modificato il valore della variabile self.sampling_period
                for i in range(10):
                    temp_sampling_period = self.sampling_period  # aggiorno il nuovo valore della variabile tempranea
                    self.send_sampling_period()  # invio il comando di cambio periodo di campionamento (reset_osc=False per default, non serve specificarlo)
                    sleep(0.01)
                print("Cambiata configurazione sampling period")
            elif (temp_trigger_level != self.trigger_level):
                for i in range(10):
                    temp_trigger_level = self.trigger_level  # aggiorno il valore della variabile temporanea con la nuova configurazione
                    self.send_trigger_level()  # invio il comando di cambio di trigger_level (reset_osc=False per default, non serve specificarlo)
                    sleep(0.01)
                print("Cambiata configurazione Trigger level")
            elif (
                    temp_trigger_type != self.trigger_type):  # controllo se l'utente ha cambiato valore della variabile self.trigger_type
                for i in range(10):
                    temp_trigger_type = self.trigger_type  # aggiorno il valore della variabile temporanea temp_trigger_type
                    self.send_trigger_type()  # invio il comando di cambio trigger (reset_osc=False per default, non serve specificarlo)
                    sleep(0.01)
                print("Cambiata configurazione Trigger type")
            sleep(0)









