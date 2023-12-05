#######################################
#   List all collections in database  #
#######################################


# Import modules
import collections
import smtplib
from pymongo import MongoClient


# Specify MongoDB conenction details
uri = "your_uri"
database = 'UBC_Dendrometer'

# Create a new client and connect to the server
client = MongoClient(uri)
db = client[database]

# Send a ping to confirm a successful connection
try:
    client.admin.command('ping')
    print("Successfully connected to MongoDB!")
except Exception as e:
    print(e)

# Get collection names in database
collections = db.list_collection_names()

# Print each collection name
for collection in collections:
    print(collection)
