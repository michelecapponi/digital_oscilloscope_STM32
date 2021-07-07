from tkinter import *
from TX_RX_Thread import *

# valori di default
# Modalita' Auto
trigger_type = 0
# Meta' dinamica
trigger_level = 128
# 1 ms
sampling_time = 1000000
trigger_position = 0
# Dimensione della griglia di visualizzazione dell'onda
screen_size = 256
# Valore massimo di tensione del segnale in ingresso (e quindi impostabile dall'utente per il trigger level)
MAX_VOLTAGE = 3.3
# Valore minimo di tensione del segnale in ingresso (e quindi impostabile dall'utente per il trigger level)
MIN_VOLTAGE = 0
# Valore minimo del tempo di campionamento impostabile
MIN_SAMPLING_PERIOD = 10000
# Valore massimo del tempo di campionamento impostabile
MAX_SAMPLING_PERIOD = 100000000

time_base = "Time base: 256 ms"

time_division = "Time division: 32 ms"

flag_transmission = 0

# Attende una connessione valida sulla porta seriale e restituisce un istanza della classe serial
# non appena viene rilevato un dispositivo sulla porta
ser = check_for_valid_connection("COM4", 115200)
reception_loop = srl_data_rec_loop(ser)
transmission_loop = srl_data_write_loop(ser)
transmission_loop.start()
reception_loop.start()


# Funzione utilizzata per la creazione del frame per stampare l'onda a schermo
def print_channel():
    # Utilizzo la variabile globale trigger_position
    global trigger_position
    global time_division
    global time_base
    global flag_transmission

    # Creo e posiziono la finestra per la visualizzazione dei canali dell'oscilloscopio
    printed_wave = Canvas(oscilloscope_frame, width=screen_size + 90, height=screen_size + 90, bg="white")
    printed_wave.grid(row=0, column=0, sticky=N)

    # Creo la casella di testo
    inserted_text = Text(window_oscilloscope, height=1)

    # Se ho ricevuto dei campioni dalla scheda, aggiorno la Time Base e la Time Division
    if flag_transmission == 1:
        # Calcolo la Time Base e la Time Division in base ai valori inseriti dall'utente
        if sampling_time_type.get() == "TE":
            if sampling_time_input.get().isdigit():
                time_base = "Time base: " + sampling_time_input.get() + " " + measurement_unit.get()
                time_division = "Time division: " + str(
                    (int(sampling_time_input.get()) / 256) * 32) + " " + measurement_unit.get()


        else:
            if sampling_time_input.get().isdigit():
                time_base = "Time base: " + str(int(sampling_time_input.get()) * 256) + " " + measurement_unit.get()
                time_division = "Time division: " + str(
                    (int(sampling_time_input.get())) * 32) + " " + measurement_unit.get()
        flag_transmission = 0


    # Creo ed inserisco la casella di testo per la Time Base
    time_base_text = Text(oscilloscope_frame, height=1, width=30)
    time_base_text.insert(END, time_base)
    time_base_text.config(state=DISABLED)
    time_base_text.grid(row=2, column=0, sticky=W)

    # Creo ed inserisco la casella di testo per la Time Division
    time_division_text = Text(oscilloscope_frame, height=1, width=30)
    time_division_text.insert(END, time_division)
    time_division_text.config(state=DISABLED)
    time_division_text.grid(row=3, column=0, sticky=W)

    # Creo ed inserisco la casella di testo per la Voltage Division
    voltage_text = Text(oscilloscope_frame, height=1, width=30)
    voltage = "Voltage division: 0,4125 V"
    voltage_text.insert(END, voltage)
    voltage_text.config(state=DISABLED)
    voltage_text.grid(row=4, column=0, sticky=W)

    # Variabile per il posizionamento della griglia, cui ogni quadrato misura 32 x 32 pixel
    grid_scale = 32
    for i in range(7):
        # Disegno le linee verticali della griglia
        printed_wave.create_line(45+grid_scale, 45, 45+grid_scale, screen_size+45, fill='seashell3')
        # Disegno le linee orizzontali della griglia
        printed_wave.create_line(45, 45+grid_scale, screen_size+45, grid_scale + 45, fill='seashell3')
        grid_scale = grid_scale + 32

    # Disegno la linea verticale nera sx della finestra di visualizzazione
    printed_wave.create_line(44, 44, 44, screen_size + 46, fill='black')
    # Disegno la linea verticale nera dx della finestra di visualizzazione
    printed_wave.create_line(45 + screen_size, 44, 45 + screen_size, screen_size + 46, fill='black')
    # Disegno la linea orizzontale superiore della finestra di visualizzazione
    printed_wave.create_line(44, 44, screen_size + 46, 44, fill='black')
    # Disegno la linea orizzontale inferiore della finestra di visualizzazione
    printed_wave.create_line(44, 46+screen_size, 46+screen_size, 46+screen_size, fill='black')

    # Creo un testo per il valore inserito dall'utente del trigger position
    if trigger_position_input.get():
        try:
            trigger_position = int(trigger_position_input.get())
            if trigger_position > 128 or trigger_position < -127:
                text = "Error in Trigger Position option!"
                trigger_position = 0
            else:
                if trigger_position < 0:
                    text = "Trigger Position:" + str(trigger_position)
                else:
                    text = "Trigger Position:+" + str(trigger_position)
        except ValueError:
            text = "Please enter only integer trigger position!"


        # Inserisco la casella di testo precedentemente creata
        inserted_text.insert(END, text)
        inserted_text.config(state=DISABLED)
        inserted_text.grid(row=1, column=0, sticky=W)

    # Creo ed inserisco la variabile per scrivere quale canale si sta visualizzando
    wave = "Wave " + C_value.get()
    printed_wave.create_text(2, 295 + 45, anchor=SW, text=wave)

    # Linea di trigger position
    printed_wave.create_line(screen_size/2 + 45, 45, screen_size/2 + 45, screen_size + 45, fill='red')
    # Linea di trigger level
    printed_wave.create_line(45, (screen_size - trigger_level) + 45, 45 + screen_size, (screen_size - trigger_level) + 45, fill='red3')

    # Creo una lista vuota in cui verra' messa la corretta formattazione per Tkinter dell'onda da stampare (canale 1)
    wave_channel1 = []
    # Creo una lista vuota in cui verranno messi i campioni ricevuti (canale 1)
    y1 = []
    # Metto nella lista y1 i campioni ricevuti
    y1 = reception_loop.ch1_data
    # Creo una lista vuota in cui verranno messi i campioni da visualizzare in base al trigger poition
    y_channel_1_visualized = []

    # Creo una lista vuota in cui verra' messa la corretta formattazione per Tkinter dell'onda da stampare (canale 2)
    wave_channel2 = []
    # Creo una lista vuota in cui verranno messi i campioni ricevuti (canale 2)
    y2 = []
    # Metto nella lista y2 i campioni ricevuti
    y2 = reception_loop.ch2_data
    # Creo una lista vuota in cui verranno messi i campioni da visualizzare in base al trigger poition
    y_channel_2_visualized = []

    # Se la lunghezza delle liste che accolgo
    if len(y2) != 0 and len(y1) != 0:
        # In base al trigger position inserito dall'utente, scelgo quali valori dell'onda visualizzare
        y_channel_1_visualized = y1[(127 + trigger_position):(383 + trigger_position)]

        y_channel_2_visualized = y2[(127 + trigger_position):(383 + trigger_position)]

        for i in range(45, screen_size + 45):
            # E metto i campioni da visualizzare nelle liste finali con le relative x
            wave_channel1.append(i)
            wave_channel2.append(i)
            wave_channel1.append(screen_size - y_channel_1_visualized[i - 45] + 45)
            wave_channel2.append(screen_size - y_channel_2_visualized[i - 45] + 45)
        # In base al canale selezionato, visualizzo l'onda corretta
        if C_value.get() == "C1":
            printed_wave.create_line(wave_channel1, fill='blue')
        elif C_value.get() == "C2":
            printed_wave.create_line(wave_channel2, fill='green')
        elif C_value.get() == "C1/C2":
            printed_wave.create_line(wave_channel1, fill='blue')
            printed_wave.create_line(wave_channel2, fill='green')

# Funzione per il refresh in tempo reale dell'onda ricevuta
def refresh_channel():
    print_channel()
    window_oscilloscope.after(100, refresh_channel)


# Funzione per il controllo e l'acquisizione dei dati inseriti da utente
def enter_data(*args):
    # Creo la casella di testo
    inserted_text = Text(window_oscilloscope, height=1)
    # Imposto il valore di default (tutti i dati corretti) della casella di testo
    text = "Data inserted correctly"
    # Uso le variabili globali
    global trigger_type
    global trigger_level
    global sampling_time
    global flag_transmission
    # Faccio dei controlli sui dati inseriti da utente, che devono rispettare determinate specifiche
    if trigger_type_input.get():
        trigger_type = int(trigger_type_input.get())
        transmission_loop.trigger_type = trigger_type
        flag_transmission = 1

    if trigger_level_input.get():
        try:
            trigger_level_in = float(trigger_level_input.get())
        except ValueError:
            text = "Enter a floating point number for trigger level!"
        else:
            if (trigger_level_in< MIN_VOLTAGE) or (trigger_level_in > MAX_VOLTAGE):
                text = "Enter a trigger level in range [0, 3.3] V!"
            else:
                quantization_size = MAX_VOLTAGE / (screen_size + 1)
                trigger_level = int(trigger_level_in / quantization_size)
                transmission_loop.trigger_level = trigger_level
                flag_transmission = 1

    if sampling_time_input.get():
        try:
            sampling_period = float(sampling_time_input.get())
        except ValueError:
            text = "Enter a floating point number for time extension!"
        else:
            time_dictionary = {"s": 1000000000, "ms": 1000000, "us": 1000, "ns": 1}
            time_unit = time_dictionary[measurement_unit.get()]
            sampling_time = int(sampling_period * time_unit)

            if sampling_time_type.get() == "TE":
                sampling_time = int(sampling_time / screen_size)

            if (sampling_time < MIN_SAMPLING_PERIOD) or (sampling_time > MAX_SAMPLING_PERIOD):
                text = "Enter a sampling time in range [0.1, 0.00001] s!"
            else:
                transmission_loop.sampling_period = sampling_time
                flag_transmission = 1

    # Inserisco la casella di testo precedentemente creata col testo corretto
    inserted_text.insert(END, text)
    inserted_text.config(state=DISABLED)
    inserted_text.grid(row=0, column=0, sticky=W)

# Funzione richiamata dal tasto di Quit per terminare l'esecuzione del programma
def quit_function():
    transmission_loop.stop_flag = 1
    reception_loop.stop_flag = 1
    window_oscilloscope.destroy()

# Funzione per il reset dei dati (trigger level, trigger type e sampling period)
def reset_data(*args):
    global trigger_type
    global trigger_level
    global sampling_time
    global trigger_position
    global sampling_time_type
    global trigger_type_input
    global time_base
    global time_division
    # Reimpostazione valori di default
    trigger_type = 0
    trigger_level = 128
    sampling_time = 1000000
    trigger_position = 0
    time_base = "Time base: 256 ms"
    time_division = "Time division: 32 ms"
    transmission_loop.trigger_type = trigger_type
    transmission_loop.trigger_level = trigger_level
    transmission_loop.sampling_period = sampling_time


# Creo il frame principale per la finestra, rendendola non modificabile in dimensioni dall'utente ed inserendo un titolo
window_oscilloscope = Tk()
window_oscilloscope.resizable(False, False)
window_oscilloscope.title("Digital Storage Oscilloscope")

# Creo il frame per i comandi
command_frame = Frame(window_oscilloscope)
command_frame.grid(column=0, row=3, sticky=(N, W, E, S))

# Creo il frame per la visualizzazione delle onde
oscilloscope_frame = Frame(window_oscilloscope)
oscilloscope_frame.grid(column=0, row=2, sticky=(N, W, E, S))

# Creo i radiobutton per i due canali
C_value = StringVar(value="C1")

Radiobutton(oscilloscope_frame, text="Channel 1", variable=C_value, value="C1").grid(row=1, column=0, sticky=W)
Radiobutton(oscilloscope_frame, text="Channel 2", variable=C_value, value="C2").grid(row=1, column=0, sticky=W, padx=80)
Radiobutton(oscilloscope_frame, text="Channels", variable=C_value, value="C1/C2").grid(row=1, column=0, sticky=W, padx=160)

# Creo la casella in cui inserire la trigger position
trigger_position_input = Entry(oscilloscope_frame, width=10, border=3)
trigger_position_input.grid(row=5, column=0, sticky=W, padx=100)

# Creo il label per il trigger position
Label(oscilloscope_frame, text="Trigger position:", font=("Times New Roman", 10)).grid(row=5, column=0, sticky=W)

# Creo i radiobutton per il trigger type
trigger_type_input = StringVar(value="00")
Radiobutton(command_frame, text="Auto", variable=trigger_type_input, value="00").grid(row=1, column=1, sticky=W)
Radiobutton(command_frame, text="Normal", variable=trigger_type_input, value="01").grid(row=2, column=1, sticky=W)
Radiobutton(command_frame, text="Single", variable=trigger_type_input, value="02").grid(row=3, column=1, sticky=W)
Radiobutton(command_frame, text="Stop", variable=trigger_type_input, value="03").grid(row=4, column=1, sticky=W)

# Creo la casella di input per il trigger level
trigger_level_input = Entry(command_frame, width=10, border=3)
trigger_level_input.grid(row=1, column=3, sticky=W)

# Creo i radiobutton per la scelta di inserimento tra Time Extent e Sampling Period
sampling_time_type = StringVar(value="SP")
Radiobutton(command_frame, text="Sampling Period", variable=sampling_time_type, value="SP").grid(row=1, column=5, sticky=W)
Radiobutton(command_frame, text="Time Extent", variable=sampling_time_type, value="TE").grid(row=1, column=6, sticky=W)

# Creo la casella di input per il sampling period
sampling_time_input = Entry(command_frame, width=10, border=3)
sampling_time_input.grid(row=1, column=7, sticky=W)

# Creo i radiobutton per l'unita' di misura
measurement_unit = StringVar(value="ns")
Radiobutton(command_frame, text="s", variable=measurement_unit, value="s").grid(row=1, column=8, sticky=W)
Radiobutton(command_frame, text="ms", variable=measurement_unit, value="ms").grid(row=2, column=8, sticky=W)
Radiobutton(command_frame, text="us", variable=measurement_unit, value="us").grid(row=3, column=8, sticky=W)
Radiobutton(command_frame, text="ns", variable=measurement_unit, value="ns").grid(row=4, column=8, sticky=W)

# Creo i label per le finestre di input
Label(command_frame, text="Trigger Type:", font=("Times New Roman", 10)).grid(row=1, column=0, sticky=W)
Label(command_frame, text="Trigger Level:", font=("Times New Roman", 10)).grid(row=1, column=2, sticky=W)

# Creo un button con la funzione di acquisizione dati ed invio alla seriale
enter_button = Button(command_frame, text="Send data", command=enter_data, border=3)
enter_button.grid(row=6, column=0, sticky=W)

# Creo un button per fare il quit del programma
quit_button = Button(command_frame, text="Quit", command=quit_function, border=3)
quit_button.grid(row=6, column=2, sticky=W)

# Creo un button per il reset
reset_button = Button(command_frame, text="Reset", command=reset_data, border=3)
reset_button.grid(row=6, column=1, sticky=W)

# Creo delle "shortcut" per i comandi, cosi' che premendo invio mando i dati
window_oscilloscope.bind('<Return>', enter_data)

# Cl cursore della keybord si va a posizionare inizialmente sul trigger level
trigger_level_input.focus()

# Così richiamo la funzione refresh ogni 100 ms
window_oscilloscope.after(100, refresh_channel)

# attivo il ciclo degli eventi
window_oscilloscope.mainloop()

