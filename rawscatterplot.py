###########################################
#   Plot combined raw data - scatterplot  #
###########################################


# Import modules
import time
import os
import csv
import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime

# Specify columns to read from
column_time_local = "time_local"
column_um = "um"

# Set data folder path and save folder names
data_path = "C:/Users/kimda/Onedrive/Documents/PhaenoFlex/data/2023/magnetic_SD"
folder_list = os.listdir(data_path)

#Loop through all data folders
for folder_name in folder_list:
    folder_path = os.path.join(data_path, folder_name)

    join_to_file_path = "data_combined_" + str(folder_name) + ".csv"
    file_path = os.path.join(folder_path, join_to_file_path)

    # Read and combine data from CSV file
    df = pd.read_csv(file_path, low_memory=False)

    # Extract data
    times = pd.to_datetime(df[column_time_local], format='ISO8601', errors='coerce')
    um = df[column_um]

    # Create a plot
    plt.figure(figsize=(9, 5))  # Adjust the width and height as needed
    plt.scatter(times, um, marker='o', s=1)

    # Format x-axis as dates
    plt.gcf().autofmt_xdate()

    # Set plot title and labels
    title = "raw_data_scatterplot.UBC_Dendrometer_" + folder_name
    plt.title(title)
    plt.xlabel('Timestamp.time_local')
    plt.ylabel('displacement.um')

    # Save the plot to a specific folder
    output_folder = "C:/Users/kimda/Downloads/plots"
    output_filename = "scatter_" + str(folder_name) + "_plot.png"
    output_path = f"{output_folder}/{output_filename}"
    plt.savefig(output_path)
    plt.close()
    print(f"Saved plot {folder_name}")

print("All plots saved to:", output_folder)