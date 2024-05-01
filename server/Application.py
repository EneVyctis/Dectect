import streamlit as st
import pandas as pd
import sqlite3
from streamlit_folium import folium_static
import folium
import plotly.express as px
from folium import plugins
from folium.features import DivIcon
def init_user_db():
    conn = sqlite3.connect('users.db')
    c = conn.cursor()
    # Création de la table des utilisateurs si elle n'existe pas déjà
    c.execute('''
        CREATE TABLE IF NOT EXISTS users (
            username TEXT PRIMARY KEY,
            password TEXT
        )
    ''')
    conn.commit()
    conn.close()


init_user_db()


def register_user(username, password):
    conn = sqlite3.connect('users.db')
    c = conn.cursor()
    try:
        c.execute("INSERT INTO users (username, password) VALUES (?, ?)", (username, password))
        conn.commit()
    except sqlite3.IntegrityError:
        return False
    finally:
        conn.close()
    return True

def login_user():
    if 'user' not in st.session_state or not st.session_state['user']:
        user = st.sidebar.text_input("Nom d'utilisateur", key="login_user")
        password = st.sidebar.text_input("Mot de passe", type="password", key="login_password")
        
        if st.sidebar.button("Se connecter"):
            if authenticate_user(user, password):
                st.session_state['user'] = user
                st.experimental_rerun()  # Relance l'exécution pour actualiser l'interface
            else:
                st.error("Échec de l'authentification")


def register_new_user():
    new_user = st.sidebar.text_input("Choisir un nom d'utilisateur")
    new_password = st.sidebar.text_input("Choisir un mot de passe", type="password")
    
    if st.sidebar.button("S'inscrire"):
        if register_user(new_user, new_password):
            st.success("Utilisateur enregistré avec succès. Veuillez vous connecter.")
        else:
            st.error("Ce nom d'utilisateur existe déjà ou erreur de saisie.")



def authenticate_user(username, password):
    conn = sqlite3.connect('users.db')
    c = conn.cursor()
    c.execute("SELECT * FROM users WHERE username=? AND password=?", (username, password))
    user_data = c.fetchone()
    conn.close()
    return user_data


def show_affluence_chart(data):
    # Création d'un graphique avec Plotly
    name_place = data['lieu'].iloc[0]
    fig = px.line(data, x='datetime', y='affluence', title='Affluence ' + name_place,
                  labels={'datetime': 'Date et Heure', 'affluence': 'Nombre de Visiteurs'}, height=500, width=1000, template='plotly_dark', line_shape='spline', render_mode='svg',)
    st.plotly_chart(fig, use_container_width=True)
    

def load_main_app():
    st.title("Affluence en temps réel")
    place_name = st.text_input("Rechercher un lieu")
    data = fetch_data(place_name) if place_name else pd.DataFrame()
    show_data_on_map(data, place_name)
    if not data.empty:
        show_affluence_chart(data)


def fetch_data(place_name):
    conn = sqlite3.connect('affluence.db')
    c = conn.cursor()
    c.execute("SELECT * FROM affluence WHERE lieu LIKE ?", ('%' + place_name + '%',))
    data = pd.DataFrame(c.fetchall(), columns=['id', 'lieu', 'affluence', 'latitude', 'longitude', 'datetime'])
    conn.close()
    return data


def show_data_on_map(data, place_name=""):
    conn = sqlite3.connect('affluence.db')
    c = conn.cursor()
    c.execute("""
        SELECT a.*
        FROM affluence a
        INNER JOIN (
            SELECT lieu, MAX(datetime) AS max_datetime
            FROM affluence
            GROUP BY lieu
        ) grouped_a ON a.lieu = grouped_a.lieu AND a.datetime = grouped_a.max_datetime
    """)
    
    all_data = pd.DataFrame(c.fetchall(), columns=['id', 'lieu', 'affluence', 'latitude', 'longitude', 'datetime'])
    conn.close()

    if not all_data.empty:
        # Si une recherche est effectuée, utilisez les coordonnées moyennes des résultats de recherche
        if not data.empty:
            mean_lat = data['latitude'].mean()
            mean_lon = data['longitude'].mean()
        else:
            mean_lat = all_data['latitude'].mean()
            mean_lon = all_data['longitude'].mean()

        m = folium.Map(location=[mean_lat, mean_lon], zoom_start=14 if not place_name else 16, control_scale=True, prefer_canvas=True, width='100%', height='100%', position='relative', left='0%', top='0%', overflow='hidden', z_index=0)
        
        cluster = plugins.MarkerCluster().add_to(m)
        for index, row in all_data.iterrows():
            popup_text = f"{row['lieu']}<br>Affluence: {row['affluence']}<br>Date et Heure: {row['datetime']}"
            folium.CircleMarker(location=[row['latitude'], row['longitude']],
                            radius=20,
                            weight=2,
                            color='blue',
                            fill_color='blue',
                            fill_opacity=0.8,
                            
                            popup=popup_text).add_to(m)

        # Si vous voulez le texte directement sur le marqueur, utilisez ceci:
            marker_text = f"{row['affluence']}"
            folium.Marker(location=[row['latitude'], row['longitude']],
                      icon=DivIcon(
                          icon_size=(150,36),
                          icon_anchor=(0,0),
                          html=f'<div style="font-size: 24pt">{marker_text}</div>',
                          )
                      ).add_to(m)

        folium_static(m)
    else:
        st.info("Aucun lieu trouvé pour cette recherche.")
        # Affiche une carte centrée sur une position par défaut (ici Lyon)


if 'user' in st.session_state and st.session_state['user']:
    load_main_app()  # Charge l'application principale si l'utilisateur est déjà connecté
else:
    st.title("Affluence en temps réel")
    st.sidebar.title("Options")
    app_mode = st.sidebar.selectbox("Choisir mode:", ["Se connecter", "S'inscrire"])
    if app_mode == "Se connecter":
        login_user()
    elif app_mode == "S'inscrire":
        register_new_user()


if 'user' in st.session_state and st.session_state['user']:
    if st.sidebar.button('Se déconnecter'):
        del st.session_state['user']
        st.experimental_rerun()  # Relance l'exécution pour revenir à l'écran de connexion
    if st.sidebar.button('Rafrachir'):
        st.experimental_rerun()