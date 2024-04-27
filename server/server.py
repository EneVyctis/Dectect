import socket
import threading
import csv
import time

ADRESSE = ''  # L'adresse IP sur laquelle le serveur écoute (ici, toutes les interfaces disponibles)
PORT = [8080,8000]   # Le port sur lequel le serveur écoute
listOfMacsWifi = {}
listOfMacsBluetooth = {}
# Fonction gérant chaque client
def gerer_client_wifi(client, adresse):
    print(f"Connexion de {adresse}")
    try:
        while True:
            donnees = client.recv(1024)  # Reçoit les données du client, jusqu'à 1024 octets à la fois
            if not donnees:
                print(f"Client {adresse} déconnecté.")
                return
            else:
                stringDonnees = donnees.decode('utf-8')  # Décode les données en UTF-8
                print(f"Reception de {adresse}: {stringDonnees}")
                if stringDonnees.strip().lower()[:-1] == "/quit":
                        arreter_serveur()
                        return
                elif stringDonnees.strip().lower()[:-1] == "/count":
                        count = len(listOfMacsWifi)
                        print(f"Il y a actuellement {count} personnes dans la zone")
                elif stringDonnees.strip().lower()[:-1] == "/list": 
                        for mac in listOfMacsWifi:
                                print(mac)
                else:
                    for k in range(len(stringDonnees)//17):
                        i = k*17
                        listOfMacsWifi[stringDonnees[i:i+17]] = time.time()
    except Exception as e:
        print(f"Erreur lors de la communication avec {adresse}: {e}")
    
    # Ferme la connexion avec le client
    try:
        client.close()
    except Exception as e:
        print(f"Erreur lors de la fermeture de la connexion avec {adresse}: {e}")

def gerer_client_bluetooth(client, adresse):
    print(f"Connexion de {adresse}")
    try:
        while True:
            donnees = client.recv(1024)  # Reçoit les données du client, jusqu'à 1024 octets à la fois
            if not donnees:
                print(f"Client {adresse} déconnecté.")
                return
            else:
                stringDonnees = donnees.decode('utf-8')  # Décode les données en UTF-8
                print(f"Reception de {adresse}: {stringDonnees}")
                if stringDonnees.strip().lower()[:-1] == "/quit":
                        arreter_serveur()
                        return
                elif stringDonnees.strip().lower()[:-1] == "/count":
                        count = len(listOfMacsBluetooth)
                        print(f"Il y a actuellement {count} personnes dans la zone")
                elif stringDonnees.strip().lower()[:-1] == "/list": 
                        for mac in listOfMacsBluetooth:
                                print(mac)
                else:
                    for k in range(len(stringDonnees)//17):
                        i = k*17
                        listOfMacsBluetooth[stringDonnees[i:i+17]] = time.time()
    except Exception as e:
        print(f"Erreur lors de la communication avec {adresse}: {e}")
    
    # Ferme la connexion avec le client
    try:
        client.close()
    except Exception as e:
        print(f"Erreur lors de la fermeture de la connexion avec {adresse}: {e}")

# Fonction actualisant la liste des macs
def actualiser_macs():
     while True:
        time.sleep(20)
        t = time.time()
        # Filtrer les clients inactifs
        macs_inactifs_wi = [mac for mac in listOfMacsWifi if listOfMacsWifi[mac] < t - 40]
        macs_inactifs_blue = [mac for mac in listOfMacsBluetooth if listOfMacsBluetooth[mac] < t - 40]
        for mac in macs_inactifs_wi:
            print(f"Supprime {mac} de la liste des MACs")
            listOfMacsWifi.pop(mac)
        for mac in macs_inactifs_blue:
            print(f'Supprime {mac} de la liste des MACs')
            listOfMacsBluetooth.pop(mac)
# Fonction pour arrêter le serveur
def arreter_serveur():
    print("Arrêt du serveur en cours...")
    for client in threading.enumerate():
        if client is threading.current_thread():
            continue
        if hasattr(client, 'shutdown'):
            client.shutdown(socket.SHUT_RDWR)
            client.close()
    serveurBlutooth.close()
    serveurWifi.close()

def ManageWifi():
    while True:
        try:
            # Accepte la connexion entrante et récupère le socket client et l'adresse du client
            client, adresseClient = serveurWifi.accept()
            # Crée un thread pour gérer ce client
            thread_client = threading.Thread(target=gerer_client_wifi, args=(client, adresseClient))
            # Démarre le thread
            thread_client.start()
        except Exception as e:
            print(f"Erreur lors de la gestion d'une connexion entrante: {e}")

def ManageBluetooth():
    while True:
        try:
            # Accepte la connexion entrante et récupère le socket client et l'adresse du client
            client, adresseClient = serveurBlutooth.accept()
            # Crée un thread pour gérer ce client
            thread_client = threading.Thread(target=gerer_client_bluetooth, args=(client, adresseClient))
            # Démarre le thread
            thread_client.start()
        except Exception as e:
            print(f"Erreur lors de la gestion d'une connexion entrante: {e}")

# Crée un objet socket pour le serveur
serveurWifi = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serveurBlutooth = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
try:
    # Associe le serveur à l'adresse et au port spécifiés
    serveurWifi.bind((ADRESSE, PORT[0]))
    serveurBlutooth.bind((ADRESSE, PORT[1]))
    # Commence à écouter les connexions entrantes, en limitant à 5 clients
    serveurWifi.listen(5)
    serveurBlutooth.listen(5)
    print("Serveurs en attente de connexions...")
except Exception as e:
    print(f"Erreur lors du démarrage du serveur: {e}")
    serveurWifi.close()
    serveurBlutooth.close()
    exit()
# Boucle principale pour accepter les connexions entrantes
thread_manage_list = threading.Thread(target = actualiser_macs)
thread_manage_list.start()

thread_manage_wifi = threading.Thread(target = ManageWifi)
thread_manage_wifi.start()

thread_manage_bluetooth = threading.Thread(target = ManageBluetooth)
thread_manage_bluetooth.start()