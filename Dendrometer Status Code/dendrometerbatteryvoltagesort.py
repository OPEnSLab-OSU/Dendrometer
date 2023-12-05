##########################################################
#   Sort collections in ascending battery voltage order  #
##########################################################

# You will need an active mongodb service running on your device to run the command
# Also you need to install pymongo: pip install pymongo
# Run this script with the following command in your command-line: python dendrometerbatteryvoltagesort.py 
# Replace fields **redacted** below with appropriate information

# Import modules
import smtplib
from pymongo import MongoClient
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart

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

# We will add report content to these variable
report = []
unsorted_list = []

# Create a while loop to iterate for each collection
i = 1
while i < 73:
  
    collection_number = i

    # Update the collection name
    collection_name = "UBC_Dendrometer_" + str(collection_number)

    # Save entire document to variable to check other fields
    collection = db[collection_name]
    last_data = collection.find_one(sort=[('_id', -1)])
    
    # Save voltage value of dendrometer
    voltage = last_data['Analog']['Vbat']

    # Append voltage values to list
    unsorted_list.append({"name": collection_name, "voltage": voltage, "warning": []})

    # Increment collection number
    i += 1

# End connection to MongoDB
client.close()

# Sort list in ascending order of voltage values! I love python <3
sorted_list = sorted(unsorted_list, key=lambda x: x['voltage'])

for dendrometer in sorted_list:

    # Extract float value for comparison
    voltage = dendrometer['voltage']

    # Check battery voltage lower boundary
    if voltage > 3.6 :
        pass 
    else: 
        dendrometer['warning'].append("Battery voltage LOW.")

    # Check battery voltage higher boundary
    if voltage < 4.3 :
        pass 
    else: 
        dendrometer['warning'].append("Battery voltage HIGH.")

# Convert dendrometers to formatted strings
formatted_dendrometers = []
for dendrometer in sorted_list:
    formatted_dendrometer = f"{dendrometer['name']}, voltage: {dendrometer['voltage']}, warnings: {dendrometer['warning']}"
    formatted_dendrometers.append(formatted_dendrometer)

# Create email content
email_content = "Ordered dendrometer battery voltage values:\n\n" + "\n\n".join(formatted_dendrometers)

# Configure email parameters
sender_email = '**YOUR_EMAIL_ADDRESS**'
receiver_emails = ['**RECEIPIENT_EMAILS**', '**RECEIPIENT_EMAILS**', '**RECEIPIENT_EMAILS**']
subject = 'Dendrometer Voltage Report'

# Set up smtp
smtp_server = 'smtp-mail.outlook.com'
smtp_port = 587
smtp_username = '**YOUR_EMAIL_ADDRESS**'
smtp_password = '**PASSWORD**'

# Send the email
try:
    with smtplib.SMTP(smtp_server, smtp_port) as server:
        server.starttls()
        server.login(smtp_username, smtp_password)
        for receiver_email in receiver_emails:
            # Create the MIMEText email message
            email_message = MIMEMultipart()
            email_message['Subject'] = subject
            email_message['From'] = sender_email
            email_message['To'] = receiver_email
            email_message.attach(MIMEText(email_content, 'plain'))

            server.send_message(email_message)
            print(f"Successfully sent email to {receiver_email}!")
        print("Successfully sent email to all receipients!")

except smtplib.SMTPException as e:
    print(f"An error occured while sending the email: {str(e)}")

