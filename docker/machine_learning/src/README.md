# Open crontab editor for current user
crontab -e

# Add this line to run retrain.sh every day at midnight (adjust path as necessary)
0 0 * * * /path/to/retrain.sh >> /path/to/logfile.log 2>&1 