# Setting up Grafana_EE4216 in your environment
## For Windows Users
**Please use WSL2 for the steps below.**<br>
1. Ensure WSL2 is updated
```bash
sudo apt-get update
sudo apt upgrade -y
```
2. Download dockers
```bash
sudo apt-get install docker -y
```
3. Start dockers
```bash
sudo systemctl start docker
```
**To use a GUI, download [docker desktop for Windows](https://www.docker.com/products/docker-desktop/)**<br>
If your docker desktop is unable to access your docker from WSL2, do the following:
1. docker desktop may prompt you to turn on WSL2 during installation.
2. Start docker desktop from Windows menu.
3. Navigate to **Settings**
4. From the **General** tab, select use **WSL 2 based engine**.
5. Select **Apply & Restart**
## For Mac Users
1. Ensure WSL2 is updated
```bash
brew update
brew upgrade
```
2. Download dockers
```bash
brew install docker
```
3. Start dockers
```bash
brew services start docker
```
**To use a GUI, download [docker desktop for Mac](https://www.docker.com/products/docker-desktop/)**<br>
## [For ALL Users] Set up Visualisation Tool for Project (Grafana)
**Since we will be deploying a few docker containers (Grafana, database etc.), we will utilise a tool called *docker compose* to deploy all containers at once in a common network.**<br>
1. Ensure you're in the following directory path:<br>
*<your_path>/EE4216-Project/docker*
2. Set up containers and pull images from cloud:
```bash
docker-compose up -d
```
3. It will take some time for the images to be pulled and loaded. Once done, you may run the following command which will show you your active docker containers:
```bash
docker ps
```
Alternatively, you may use **docker desktop** app to view your running containers
### Miscellaneous Notes:
- To stop all containers from running, execute the *stop.sh* script
```bash
./stop.sh
```
- To start all containers again, execute the *start.sh* script
```bash
./start.sh
```
- To restart all containers, execute the *restart-server.sh* script
```bash
./restart-server.sh
```
- To destroy all containers, execute the *destroy_all_containers.sh* script
```bash
./destory_all_containers.sh
```
- To create all containers again, execute the *create_all_containers.sh* script
```bash
./create_all_containers.sh
```
- Find out more about **docker products** from their website/other sources.