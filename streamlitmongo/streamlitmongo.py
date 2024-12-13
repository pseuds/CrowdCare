import streamlit as st
import pymongo
from pymongo import MongoClient
import pandas as pd
import time
from datetime import datetime
from urllib.parse import quote_plus
import plotly.express as px
import plotly.graph_objects as go

# Configuration and constants
REFRESH_INTERVAL = 5  # refresh every 5 seconds
#TODO: CHANGE DB NAME AND COLLECTION ACCORDINGLY
DEFAULT_DB = ""
DEFAULT_COLLECTION = ""

# Initialize MongoDB connection
@st.cache_resource
def init_connection():
    try:
        #TODO: UNAME AND PASSWD FOR MONGODB
        username = quote_plus("")
        password = quote_plus("")
        #TODO: MONGODB CONNECTION URL
        MONGO_URI = ""
        # Test the connection
        mongo_client = MongoClient(MONGO_URI)
        db = mongo_client.humidity_sensor
        collection = db.alldata
        return mongo_client

    except Exception as e:
        st.error(f"Failed to connect to MongoDB: {str(e)}")
        return None

# Fetch data from collection with error handling
def get_data(client, database_name, collection_name):
    try:
        db = client[database_name]
        collection = db[collection_name]
        data = list(collection.find(
            {},
            {
                '_id': 0,
                'timestamp': 1,
                'humidity': 1,
                'temperature': 1,
                'motion': 1
            }
        ).sort('timestamp', -1).limit(100))

        if not data:
            st.warning("No data found in the collection")
            return pd.DataFrame()
        
        df = pd.DataFrame(data)
        df['timestamp'] = pd.to_datetime(df['timestamp'])
        return df
    except Exception as e:
        st.error(f"Error fetching data: {str(e)}")
        return pd.DataFrame()

def create_chart(df, columns, chart_type):
    fig = go.Figure()
    
    for col in columns:
        fig.add_trace(
            go.Scatter(
                x=df.index,
                y=df[col],
                mode='lines',
                name=col
            )
        )
    
    fig.update_layout(
        height=300,  # Reduced height for better fit
        margin=dict(l=0, r=0, t=30, b=0),
        showlegend=True,
        legend=dict(
            orientation="h",
            yanchor="bottom",
            y=1.02,
            xanchor="right",
            x=1
        )
    )
    
    return fig

def main():
    st.set_page_config(page_title="Sensor Data Visualization", layout="wide")
    
    st.sidebar.header("Configuration")
    refresh_rate = st.sidebar.slider("Refresh Rate (seconds)", 1, 60, 5)
    
    st.title("Real-Time Sensor Data Visualization")
    
    status_container = st.empty()
    col1, col2, col3 = st.columns(3)
    
    with col1:
        humidity_container = st.empty()
    with col2:
        temperature_container = st.empty()
    with col3:
        motion_container = st.empty()

    raw_data_container = st.empty()
    
    client = init_connection()

    if client:
        try:
            while True:
                # Generate unique timestamp for this iteration
                timestamp = int(time.time())
                
                df = get_data(client, "humidity_sensor", "alldata")

                if not df.empty:
                    status_container.write(
                        f"Last updated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}"
                    )
                    
                    with humidity_container.container():
                        st.subheader("Humidity Data")
                        fig_humidity = create_chart(df, ['humidity'], "Line Chart")
                        if fig_humidity:
                            st.plotly_chart(
                                fig_humidity, 
                                use_container_width=True,
                                key=f"humidity_chart_{timestamp}"
                            )
                    
                    with temperature_container.container():
                        st.subheader("Temperature Data")
                        fig_temperature = create_chart(df, ['temperature'], "Line Chart")
                        if fig_temperature:
                            st.plotly_chart(
                                fig_temperature, 
                                use_container_width=True,
                                key=f"temperature_chart_{timestamp}"
                            )
                    
                    with motion_container.container():
                        st.subheader("Motion Data")
                        fig_motion = create_chart(df, ['motion'], "Line Chart")
                        if fig_motion:
                            st.plotly_chart(
                                fig_motion, 
                                use_container_width=True,
                                key=f"motion_chart_{timestamp}"
                            )

# Add this inside your while True loop, after all your existing graph code
                    with raw_data_container.container():
                        st.subheader("Raw Sensor Data")
                        df_sorted = df.sort_values(by='timestamp', ascending=False)
                        st.dataframe(
                            df,
                            use_container_width=True,
                            key=f"raw_data_{timestamp}"
                        )                
                time.sleep(refresh_rate)
                
        except Exception as e:
            st.error(f"An error occurred: {str(e)}")
if __name__ == "__main__":
    main()
