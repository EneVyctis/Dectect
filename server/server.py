import socket

ADRESSE = ''
PORT = 8080
f = open("data.txt", "a")

serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serveur.bind((ADRESSE, PORT))
serveur.listen(1)
client, adresseClient = serveur.accept()
print(f"Connexion de {adresseClient}")

while(True):
    donnees = client.recv(1024)
    if not donnees:
            print(f"Erreur de reception.")
            break
    else:
            stringDonnees = donnees.decode('utf-8')
            print(f"Reception de: {stringDonnees}")
            f.write(stringDonnees)

print("Fermeture du fichier")
f.close()
print("Fermeture de la connexion avec le client.")
client.close()
print("Arret du serveur.")
serveur.close()
