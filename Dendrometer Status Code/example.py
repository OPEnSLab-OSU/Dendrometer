###################################################################
#   Download last document from each collection on the database   #
###################################################################


# Import modules
import csv
import smtplib
import time
from pymongo import MongoClient

# Specify MongoDB conenction details
uri = "mongodb+srv://**USERNAME**:**PASSWORD**@remotetest.cls7o.mongodb.net/UBC_Dendrometer"
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


def get_last_documents():
    last_documents = {}

    i = 1

    while i < 73:

        collection_number = i
        collection_name = "UBC_Dendrometer_" + str(collection_number)

        collection = db[collection_name]
        last_document = collection.find_one(sort=[('_id', -1)])

        if collection_name not in last_documents:
            last_documents[collection_name] = {}

        last_documents[collection_name]['_id'] = last_document['_id']
        last_documents[collection_name]['Packet.Number'] = last_document['Packet']['Number']
        last_documents[collection_name]['Timestamp.time_local'] = last_document['Timestamp']['time_local']

        i += 1

    return last_documents

def export_to_csv(last_documents):
    for collection_name, document in last_documents.items():
        if document:
            filename = "last_document_" + str(collection_name) + ".csv"
            with open(filename, mode='w', newline='') as file:
                writer = csv.DictWriter(file, fieldnames=document.keys())
                writer.writeheader()
                writer.writerow(document)

if __name__ == "__main__":

    last_docs = get_last_documents()
    client.close()

    export_to_csv(last_docs)

    current_time = time.strftime('%Y-%m-%d %H:%M:%S')

    with open('data_download_tracker.txt', 'w') as txtfile:

        txtfile.write(f"Data was last downloaded: {current_time}\n\n")

    print("All documents exported successfully.")


