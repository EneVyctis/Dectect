import socket
import threading
import csv

ADRESSE = ''  # L'adresse IP sur laquelle le serveur écoute (ici, toutes les interfaces disponibles)
PORT = 8080   # Le port sur lequel le serveur écoute
listOfMacs = {}
# Fonction gérant chaque client
def gerer_client(client, adresse):
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
                        count = len(listOfMacs)
                        print(f"Il y a actuellement {count} personnes dans la zone")
                elif stringDonnees.strip().lower()[:-1] == "/list":
                        for mac in listOfMacs:
                                print(mac)
                else:
                        listOfMacs[stringDonnees] = 1;
    except Exception as e:
        print(f"Erreur lors de la communication avec {adresse}: {e}")
    
    # Ferme la connexion avec le client
    try:
        client.close()
    except Exception as e:
        print(f"Erreur lors de la fermeture de la connexion avec {adresse}: {e}")

# Fonction pour arrêter le serveur
def arreter_serveur():
    print("Arrêt du serveur en cours...")
    serveur.close()
    print("Serveur arrêté.")

def consulter_nombre():
        with open("data.csv", "r", newline='') as f:
                reader = csv.reader(f)
                count = sum(1 for line in reader)
                return count

# Crée un objet socket pour le serveur
serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Associe le serveur à l'adresse et au port spécifiés
    serveur.bind((ADRESSE, PORT))
    # Commence à écouter les connexions entrantes, en limitant à 5 clients
    serveur.listen(5)
    print("Serveur en attente de connexions...")
except Exception as e:
    print(f"Erreur lors du démarrage du serveur: {e}")
    serveur.close()
    exit()

# Boucle principale pour accepter les connexions entrantes
while True:
    try:
        # Accepte la connexion entrante et récupère le socket client et l'adresse du client
        client, adresseClient = serveur.accept()
        # Crée un thread pour gérer ce client
        thread_client = threading.Thread(target=gerer_client, args=(client, adresseClient))
        # Démarre le thread
        thread_client.start()
    except Exception as e:
        print(f"Erreur lors de la gestion d'une connexion entrante: {e}")

# Ferme le socket du serveur (ce code ne sera jamais atteint dans ce contexte)
serveur.close()