########################################
#   Combine data files into one file   #
########################################


# Import modules
import os
import csv

# Set data folder path and save folder names
data_path = "C:/Users/kimda/Onedrive/Documents/PhaenoFlex/data/2023/magnetic_SD"
folder_list = os.listdir(data_path)

# Initialize an empty list to hold DataRows
data_rows = []

# Loop through all dendrometer folders
for folder_name in folder_list:
    folder_path = os.path.join(data_path, folder_name)

    # List all csv files in each folder
    file_list = os.listdir(folder_path)

    # Initialize an empty list to hold DataRow tuples for this folder
    data_rows = []
    header_flag = False

    # Read and combine data from CSV files in folder
    for file_name in file_list:
        file_path = os.path.join(folder_path, file_name)

        with open(file_path, 'r') as csvfile:

            csv_reader = csv.reader(csvfile)

            if not header_flag:
                for _ in range(2):  # Skip 2 rows
                    next(csv_reader)
                header_flag = True
            else:
                for _ in range(4):  # Skip 4 rows
                    next(csv_reader)

            for row in csv_reader:
                data_rows.append(row)  # Convert each row to a tuple

    # Specify the destination path for the combined CSV file
    combined_name = "data_combined_" + str(folder_name) + ".csv"
    combined_file_path = os.path.join(folder_path, combined_name)

    #os.remove(combined_file_path)

    # Save the combined DataRow tuples to a new CSV file
    with open(combined_file_path, 'w', newline='') as csvfile:
        csv_writer = csv.writer(csvfile)
        csv_writer.writerows(data_rows)  # Write the DataRow tuples

    print("Combined data saved to:", combined_file_path)

# Finished!
print("All data combined successfully.")
