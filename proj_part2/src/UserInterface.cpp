/*
 * UserInterface.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: epassos
 */

#include "UserInterface.h"
#include "Reservation.h"
#include "Passenger.h"
#include <cstdlib>
#include "stringMatch.h"
#include <fstream>
#include <istream>
#include <string>
#include <sstream>

#define HOTELS_FILENAME "hotels.txt"
#define RESERVATIONS_FILENAME "reservations.txt"
#define NUMBER_VANS 2

inline void hold(){
	cout<<"\nPress Enter to continue...";
	cin.ignore();
	cin.get();
}


UserInterface::UserInterface(Graph *g, int maxPassengers, Node *source) {
	this->graph = g;
	this->maxPassengers = maxPassengers;
	this->source = source;
	this->isPlanned = false;

	maxLat = g->vertexSet[0]->getCoordinates().getLatitude();
	minLat = g->vertexSet[0]->getCoordinates().getLatitude();
	maxLng = g->vertexSet[0]->getCoordinates().getLongitude();
	minLng = g->vertexSet[0]->getCoordinates().getLongitude();
}

void UserInterface::readHotels() {
	ifstream file;
	file.open(HOTELS_FILENAME);

	if (!file.is_open())
		return;

	while (!file.eof()) {

		string buff;
		buff.clear();
		stringstream ss;

		string name;
		unsigned long idNode;

		if (getline(file, buff, ';')) {
			ss << buff;
			ss >> idNode;
			ss.clear();
		}

		if (!getline(file, name, ';')) {
			break;
		}

		Node *n = graph->getNode(idNode);

		if (n == NULL) {
			cout << "Node not found: " << idNode << endl;
			continue;
		}

		Hotel h(name, n);
		hotels.push_back(h);

	}

	assignHotelsToVans();
}

void UserInterface::printHotels() {
	cout << endl;
	for (unsigned int i = 0; i < hotels.size(); i++) {
		cout << i + 1 << " - " << hotels[i].name << endl;
	}
}

void UserInterface::readReservations() {
	ifstream file;

	file.open(RESERVATIONS_FILENAME);

	if (!file.is_open())
		return;

	while (!file.eof()) {

		string buff;
		buff.clear();
		stringstream ss;

		string name;
		int nif;
		unsigned int hotel;
		string arrival_time;

		getline(file, name, ';');

		if (getline(file, buff, ';')) {
			ss << buff;
			ss >> nif;
			ss.clear();
		}

		if (getline(file, buff, ';')) {
			ss << buff;
			ss >> hotel;
			ss.clear();
		}

		if (getline(file, buff, '\n')) {
			ss << buff;
			ss >> arrival_time;
			ss.clear();
		}

		Passenger* p = new Passenger(name, nif);

		Reservation res(hotels[hotel - 1].node, arrival_time, p);

		addReservation(res);

	}

	file.close();
}

void UserInterface::printReservations() {
	priority_queue<Reservation> temp;

	cout << "Unassigned reservations: " << endl << endl;
	while (!reservations.empty()) {
		cout << "Name: ";
		cout << reservations.top().getClient()->getName() << endl;
		cout << "Destination: ";
		cout << reservations.top().getDestination()->getId() << endl;
		cout << "Arrival Time: ";
		cout << reservations.top().getArrivalTime() << endl << endl;

		temp.push(reservations.top());
		reservations.pop();
	}

	while (!temp.empty()) {
		reservations.push(temp.top());
		temp.pop();
	}

	cout<<"\nPassengers assigned to vans:"<<endl<<endl;

	for(unsigned int i = 0; i < vans.size(); i++){
			cout<<"\n\nVan "<<i<<endl<<endl;
		for(unsigned int j = 0; j<vans[i]->passengers.size(); j++){
			cout << "Name: ";
			cout << vans[i]->passengers[j].getClient()->getName() << endl;
			cout << "Destination: ";
			cout << vans[i]->passengers[j].getDestination()->getId() << endl<<endl;
		}
	}
}
bool UserInterface::addReservation(Reservation r) {
	priority_queue<Reservation> temp;

	string name = r.getClient()->getName();
	while (!reservations.empty()) {
		string topName = reservations.top().getClient()->getName();
		if (topName == name) {
			return false;
		}

		temp.push(reservations.top());
		reservations.pop();
	}

	while (!temp.empty()) {
		reservations.push(temp.top());
		temp.pop();
	}

	reservations.push(r);
	return true;
}

Reservation UserInterface::reservationMenu() {
	string name;
	int nif;
	unsigned int hotel;
	string arrival_time;

	cout << "Reservation Menu\n";
	cin.ignore(1000,'\n');

	cout << "Insert name: ";
	getline(cin, name);

	cout << "Insert NIF: ";
	cin >> nif;
	cin.ignore(1000, '\n');

	Node *hotelNode;

	printHotels();
	cout << "Choose Destination: ";
	cin >> hotel;
	cin.ignore(1000, '\n');
	while (hotel < 1 || hotel > hotels.size()) {
	cout << "Please choose a valid hotel: ";
		cin >> hotel;
		cin.ignore(1000, '\n');
	}

	hotelNode = hotels[hotel - 1].node;

	cout << "Insert Arrival Time: ";
	cin >> arrival_time;
	cin.ignore(1000, '\n');

	Passenger* p = new Passenger(name, nif);

	Reservation res(hotelNode, arrival_time, p);

	return res;
}

void UserInterface::transferMenu() {

	int i = 0;
	if (this->isPlanned) {

		for (unsigned int i = 0; i < vans.size(); i++) {

			vector<Node *> total_path;

			Node *currNode = source;
			Node *nextNode;

			if (vans[i]->passengers.empty())
				continue;

			while (!vans[i]->passengers.empty()) {
				vector<Node *> semi_path;
				Reservation nextPassenger = currNode->getClosestDestination(
						vans[i]->passengers);
				nextNode = nextPassenger.getDestination();

				if (nextNode != currNode) {
					semi_path = transferTo(currNode->getId(),
							nextNode->getId());
					total_path.insert(total_path.end(), semi_path.begin(),
							semi_path.end());
					semi_path.clear();
				}

				cout << "Van " << i << " transfered client "
						<< nextPassenger.getClient()->getName() << endl;

				for (unsigned int j = 0; j < vans[i]->passengers.size(); j++) {
					if (vans[i]->passengers[j].getClient()->getName()
							== nextPassenger.getClient()->getName()) { //remove the passenger from the van
						vans[i]->passengers.erase(
								vans[i]->passengers.begin() + j);
						break;
					}
				}

				currNode = nextNode;

			}

			displayGraph(total_path);
			cin.get();
			total_path.clear();
			nextNode = source;
			transferTo(currNode->getId(), nextNode->getId());
			cout << "\nBack at the airport!\n" << endl << endl;

		}
		this->isPlanned = false;
	} else
		cout << "Error! You must plan the transfer before.\n";

	return;
}

void UserInterface::mainMenu() {

	cout << "\n\n\n------------------------------------\n";
	cout << "Welcome!\n";
	cout << "Please choose an option: \n";
	cout << "1 - Add Reservation\n";
	cout << "2 - Show Reservations\n";
	cout << "3 - Plan Trip\n";
	cout << "4 - Do Trip\n";
	cout << "5 - Show Map\n";
	cout << "6 - Search Van by itinerary\n";
	cout << "7 - Search Van by client name\n";
	cout << "8 - Exit\n";
	cout << "\nYour option: ";

	int op;
	cin >> op;
	cin.ignore(1000, '\n');

	while (op < 1 || op > 8){
		cout << "Please choose a valid option: ";
		cin >> op;
	}

	vector<Node*> empty;

	switch (op) {
	case 1:
	{
		Reservation res;
		res = reservationMenu();
		addReservation(res);
		hold();
		mainMenu();
		break;
	}
	case 2:
		printReservations();
		hold();
		mainMenu();
		break;
	case 3:
		assignClientsToVans();
		hold();
		mainMenu();
		break;
	case 4:
		updateCoordinates();
		hold();
		transferMenu();
		mainMenu();
		break;
	case 5:
		updateCoordinates();
		cout << minLat << " " << maxLat << " " << minLng << " " << maxLng << endl;
		displayGraph(empty);
		mainMenu();
		break;
	case 6:
		searchVanByRoad();
		hold();
		mainMenu();
		break;

	case 7:
		searchVanByClient();
		hold();
		mainMenu();
		break;

	case 8:
		cout << "\nGoodBye!\n";
		exit(0);
	}
}

void UserInterface::displayGraph(vector<Node *> path) {
	GraphViewer *gv = new GraphViewer(600, 600, false);

	gv->createWindow(1200, 1200);
	gv->defineEdgeColor("blue");
	gv->defineVertexColor("yellow");

	//gv->setBackground("image.png");

	cout << "1\n";
	for (unsigned int i = 0; i < graph->vertexSet.size(); i++) {
		double lat = graph->vertexSet[i]->getCoordinates().getLatitude();
		double lng = graph->vertexSet[i]->getCoordinates().getLongitude();
		int x, y;

		x = floor(((lng - minLng) * 4200 / (maxLng - minLng)));
		y = 4200 - floor(((lat - minLat) * 4200 / (maxLat - minLat)));

		gv->addNode(graph->vertexSet[i]->getId(), x, y);
		gv->setVertexLabel(graph->vertexSet[i]->getId(), ".");

	}
	cout << "2\n";
	int k = 1;
	for (unsigned int i = 0; i < graph->vertexSet.size(); i++) {
		for (unsigned int j = 0; j < graph->vertexSet[i]->adj.size(); j++) {
			gv->addEdge(k, graph->vertexSet[i]->getId(),
					graph->vertexSet[i]->adj[j]->getDest()->getId(),
					EdgeType::UNDIRECTED);
			//add road names, not advised.
			//gv->setEdgeLabel(k, graph->vertexSet[i]->adj[j]->getRoad()->getName());
			k++;
		}
	}
	cout << "3\n";
	for (unsigned int i = 0; i < hotels.size(); i++) {
		gv->setVertexColor(hotels[i].node->getId(), GREEN);
		gv->setVertexLabel(hotels[i].node->getId(), hotels[i].name);
	}
	cout << "4\n";
	for (unsigned int i = 0; i < path.size(); i++) {
		gv->setVertexColor(path[i]->getId(), RED);
	}

	cout << "5\n";
	gv->rearrange();
}

vector<Node *> UserInterface::transferTo(unsigned long id_from,
		unsigned long id_dest) {
	vector<Node *> path;

	graph->resetIndegrees();
	graph->dijkstraShortestPath(id_from);

	path = graph->getPath(id_from, id_dest);
	if (path.size() == 0)
		return path;

	string oldRoadName = "1";

	for (unsigned int i = 0; i < path.size() - 1; i++) {
		for (unsigned int j = 0; j < path[i]->adj.size(); j++) {
			if (path[i]->adj[j]->getDest()->getId() == path[i + 1]->getId()) {

				if (oldRoadName != path[i]->adj[j]->getRoad()->getName()) {
					oldRoadName = path[i]->adj[j]->getRoad()->getName();
					cout << "Turn to road ";
					if (oldRoadName.size() == 0)
						cout << "Unnamed road" << endl;
					else
						cout << oldRoadName << endl;

				}
			}
		}
		//cout<<" Passing Node "<<path[i]->getId()<<endl;
	}
	return path;
}

void UserInterface::assignHotelsToVans() {

	unsigned int hotelsPerVan = hotels.size() / NUMBER_VANS;
	unsigned int indexHotel = 0;
	double distMin = INT_MAX;
	int indexDistMinMax;
	double dist;

	vector<Hotel> zone;

	unsigned int i = 0;
	while (i < NUMBER_VANS) {

		if (!hotels[indexHotel].getAssigned()) { //finds the first unassigned hotel
			i++;
			hotels[indexHotel].setAssigned(true);
			zone.push_back(hotels[indexHotel]);

			for (unsigned int j = 1; j < hotelsPerVan; j++) { //fills the vector with the closest hotels

				for (unsigned int k = 0; k < hotels.size(); k++) {
					if (k != indexHotel && !hotels[k].getAssigned()) {
						//zone.push_back(hotels[k]);
						dist = getDistance(
								hotels[indexHotel].getNode()->getPoint(),
								hotels[k].getNode()->getPoint());
						if (dist < distMin) {
							distMin = dist;
							indexDistMinMax = k;
						}
					}
				}
				hotels[indexDistMinMax].setAssigned(true);
				zone.push_back(hotels[indexDistMinMax]);
				distMin = INT_MAX;
			}
			indexHotel++;
			vans.push_back(new Van(zone));
			zone.clear();
		} else if (indexHotel < hotels.size() - 1) {
			indexHotel++;
		} else
			indexHotel = 0;
	}

	//gets the unassigned hotels (if there are any) and assigns them to the vans that serve the closest hotels

	vector<Hotel> unassignedHotels;

	for (unsigned int m = 0; m < hotels.size(); m++) {
		if (!hotels[m].getAssigned())
			unassignedHotels.push_back(hotels[m]);
	}

	int minVan;

	for (unsigned int m = 0; m < unassignedHotels.size(); m++) {
		distMin = INT_MAX;

		for (unsigned int n = 0; n < vans.size(); i++) {
			dist = getDistance(vans[n]->hotelZone[0].getNode()->getPoint(),
					unassignedHotels[m].getNode()->getPoint());

			if (dist < distMin) {
				distMin = dist;
				minVan = n;
			}
		}

		vans[minVan]->hotelZone.push_back(unassignedHotels[m]);

	}

	for (unsigned int m = 0; m < hotels.size(); m++) {
		if (!hotels[m].getAssigned())
			unassignedHotels.push_back(hotels[m]);
	}
}

void UserInterface::assignClientsToVans() {

	priority_queue<Reservation> temp;

	if (reservations.empty()) {
		cout << "No reservations!\n";
		return;
	}

	while (!reservations.empty()) {
		for (unsigned int i = 0; i < vans.size(); i++) {
			for (unsigned int j = 0; j < vans[i]->hotelZone.size(); j++) {
				if (vans[i]->hotelZone[j].getNode()->getId()
						== reservations.top().getDestination()->getId()) {
					if (vans[i]->passengers.size() < MAX_PASSENGERS) {
						vans[i]->passengers.push_back(reservations.top());
						cout << "Passenger "
								<< reservations.top().getClient()->getName()
								<< " assigned to van " << i << endl;
						reservations.pop();

					}

					else { //the van is full, the client will wait
						temp.push(reservations.top());
						reservations.pop();
					}
				}
			}
		}
	}

	while (!temp.empty()) {
		reservations.push(temp.top());
		temp.pop();
	}

	this->isPlanned = true;
}

void UserInterface::updateCoordinates() {
	for (unsigned int i = 0; i < graph->vertexSet.size(); i++) {
		double lat = graph->vertexSet[i]->getCoordinates().getLatitude();
		double lng = graph->vertexSet[i]->getCoordinates().getLongitude();

		if (maxLat < lat)
			maxLat = lat;
		if (minLat > lat)
			minLat = lat;
		if (maxLng < lng)
			maxLng = lng;
		if (minLng > lng)
			minLng = lng;
	}
}

//////////////////////////////////////
//PART 2
/////////////////////////////////////

void UserInterface::searchVanByRoad() {

	graph->dijkstraShortestPath(SOURCE_NODE_ID);
	string name;
	char tmp[256];

	cout << "Road name: ";
	cin.getline(tmp, 256, '\n');

	name = tmp;

	bool found = false;
	vector<Node *> hotelNodes;
	int min = INT_MAX;

	vector<int> closestMatchVans;
	vector<string> closestMatches;
	vector<int> exactMatchVans;
	vector<string> exactMatches;

	//get the name of the roads of the hotels served by a van and search by exact string matching
	for (unsigned int i = 0; i < vans.size(); i++) {
		hotelNodes.clear();

		for (unsigned int j = 0; j < vans[i]->getHotels().size(); j++) {
			Node* currNode = vans[i]->getHotels()[j].getNode();


		string roadName;

		//get the road the van will go through
		//for every hotel the van serves

			for (unsigned int m = 0; m < currNode->path->adj.size(); m++) { 	//get the road through which the van gets to that hotel

				if (currNode->getId() == currNode->path->adj[m]->getDest()->getId()) {

					roadName = currNode->path->adj[m]->getRoad()->getName();

					if (numStringMatching(name, roadName) != 0) {
						exactMatchVans.push_back(i);
						exactMatches.push_back(roadName);
						found = true;
					}

					if (!found) {													//No exact matches found yet
						int x = editDistance(name, roadName);						//check the edit distance of the current road name
						if (x < min) {
							min = x;
							closestMatches.clear();
							closestMatchVans.clear();
							closestMatches.push_back(roadName);
							closestMatchVans.push_back(i);
						}
						else if (x == min) {
							closestMatches.push_back(roadName);
							closestMatchVans.push_back(i);
						}
					}

					break;
				}
			}
		}
	}
	if (found) {
		cout << "Matches found:\n";
		for (unsigned int m = 0; m < exactMatches.size(); m++) {
			cout << "Van " << exactMatchVans[m] << " - " << exactMatches[m]	<< endl;
		}
	}

	else {
		cout << "No matches found, closest matching strings (Edit distance = " << min << " ):"<<endl;
		for (unsigned int m = 0; m < closestMatches.size(); m++) {
			cout << "Van " << closestMatchVans[m] << " - "<< closestMatches[m] << endl;
		}
	}
	int vanIndex = -1;
	if((found && exactMatches.size() == 1) || (!found && closestMatches.size() == 1)){
		cout << "Do you wish to travel in this van? (y/n)";
		vanIndex = found ? exactMatchVans[0] : closestMatchVans[0];
	}

	else cout << "Do you wish to travel in one of these vans? (y/n)";

	char op;
	cin >> op;
	while(op != 'y' && op != 'Y' && op != 'n' && op!= 'N'){
		cout << "Please enter a valid option. ";
		cin >> op;
	}

	if(op == 'N' || op == 'n')
		return;

	if(vanIndex == -1){
		cout << "Van number: ";
		cin >> vanIndex;
	}

	Reservation res;
	res = reservationMenu();
	if(!putClientInVan(res,vanIndex))
		cout << "Operation could not be completed\n";
}

void UserInterface::searchVanByClient(){

	if (!isPlanned){
		cout << "There are no clients in the vans yet. Please plan the trip before choosing this option\n";
		return;
	}

	string name;
	char tmp[256];

	cout<<"Name of the client: ";
	cin.getline(tmp, 256, '\n');
	name = tmp;

	bool found = false;
	int min = INT_MAX;

	vector<int> closestMatchVans;
	vector<string> closestMatches;

	vector<int> exactMatchVans;
	vector<string> exactMatches;

	//get the name of the roads of the hotels served by a van and search by exact string matching
	for(unsigned int i = 0; i < vans.size(); i++){
		for(unsigned int j = 0; j < vans[i]->passengers.size(); j++){
			string currName = vans[i]->passengers[j].getClient()->getName();
			if(numStringMatching(name,currName) != 0){
				exactMatchVans.push_back(i);
				exactMatches.push_back(currName);
				found = true;
			}

			if (!found) {													//No exact matches found yet
				int x = editDistance(name, currName);						//check the edit distance of the current road name
				if (x < min) {
					min = x;
					closestMatches.clear();
					closestMatchVans.clear();
					closestMatches.push_back(currName);
					closestMatchVans.push_back(i);
				}
				else if (x == min) {
					closestMatches.push_back(currName);
					closestMatchVans.push_back(i);
				}
			}
		}
	}

	if (found) {
		cout << "Matches found:\n";
		for (unsigned int m = 0; m < exactMatches.size(); m++) {
			cout << "Van " << exactMatchVans[m] << " - " << exactMatches[m]	<< endl;
		}
	}

	else {
		cout << "No matches found in the vans, closest matching strings (Edit distance = " << min << " ):"<<endl;
		for (unsigned int m = 0; m < closestMatches.size(); m++) {
			cout << "Van " << closestMatchVans[m] << " - "<< closestMatches[m] << endl;
		}
	}

	int vanIndex = -1;
	if((found && exactMatches.size() == 1) || (!found && closestMatches.size() == 1)){
		cout << "Do you wish to travel in this van? (y/n)";
		vanIndex = found ? exactMatchVans[0] : closestMatchVans[0];
	}

	else cout << "Do you wish to travel in one of these vans? (y/n)";

	char op;
	cin >> op;
	while(op != 'y' && op != 'Y' && op != 'n' && op!= 'N'){
		cout << "Please enter a valid option. ";
		cin >> op;
	}

	if(op == 'N' || op == 'n')
		return;

	if(vanIndex == -1){
		cout << "Van number: ";
		cin >> vanIndex;
	}

	Reservation res;
	res = reservationMenu();
	if(!putClientInVan(res,vanIndex))
		cout << "Operation could not be completed\n";
}

bool UserInterface::putClientInVan(Reservation client, int vanIndex){
	if(vanIndex < 0 || vanIndex > vans.size()){
		cout << "Selected van does not exist.\n";
		return false;
	}

	if(vans[vanIndex]->passengers.size() >= MAX_PASSENGERS){
		cout << "Selected van is full.\n";
		return false;
	}

	for(unsigned int i = 0; i < vans[vanIndex]->hotelZone.size(); i++){
		if(client.getDestination()->getId() == vans[vanIndex]->hotelZone[i].getNode()->getId()){
			vans[vanIndex]->passengers.push_back(client);
			return true;
		}
	}

	cout << "The van you selected does not go to your destination.\n";
	return false;
}
