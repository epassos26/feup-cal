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

#include <fstream>
#include <istream>
#include <string>
#include <sstream>

#define HOTELS_FILENAME "hotels.txt"
#define RESERVATIONS_FILENAME "reservations.txt"
#define NUMBER_VANS 2

Hotel::Hotel(string name, Node *n) {
	this->name = name;
	this->node = n;
	this->assigned = false;
}

bool Hotel::getAssigned() {
	return this->assigned;
}

void Hotel::setAssigned(bool flag) {
	this->assigned = flag;
}

string Hotel::getName() const {
	return name;
}

Node *Hotel::getNode() const {
	return node;
}

Van::Van(vector<Hotel*> hz) {
	this->hotelZone = hz;
}

UserInterface::UserInterface(Graph *g, int maxPassengers, Node *source) {
	this->graph = g;
	this->maxPassengers = maxPassengers;
	this->source = source;

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

		Hotel* h = new Hotel(name, n);
		hotels.push_back(h);

	}

	assignHotelsToVans();

	cout << vans.size() << endl;
	for (unsigned int i = 0; i < vans.size(); i++) {
		cout << vans[i]->hotelZone[0]->getNode()->getId() << endl;
		cout << vans[i]->hotelZone[1]->getNode()->getId() << endl;
	}

	cin.get();
}

void UserInterface::printHotels() {
	cout << endl;
	for (unsigned int i = 0; i < hotels.size(); i++) {
		cout << i + 1 << " - " << hotels[i]->name << endl;
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

		cout << name << " " << nif << " " << hotel << " " << arrival_time
				<< endl;
		Passenger* p = new Passenger(name, nif);

		Reservation res(hotels[hotel - 1]->node, arrival_time, p);

		addReservation(res);

	}
}

void UserInterface::printReservations() {
	priority_queue<Reservation> temp;

	cout << "Reservations: " << endl << endl;
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

	cout << "Press Enter to Continue...";
	cin.get();

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

void UserInterface::reservationMenu() {
	string name;
	int nif;
	unsigned int hotel;
	string arrival_time;

	cout << "Reservation Menu\n";

	cout << "Insert name: ";
	getline(cin, name);

	cout << "Insert NIF: ";
	cin >> nif;
	cin.ignore(1000, '\n');

	printHotels();
	cout << "Choose Destination: ";
	cin >> hotel;
	cin.ignore(1000, '\n');
	while (hotel < 1 || hotel > hotels.size()) {
		cout << "Please choose a valid hotel: ";
		cin >> hotel;
		cin.ignore(1000, '\n');
	}

	cout << "Insert Arrival Time: ";
	cin >> arrival_time;
	cin.ignore(1000, '\n');

	Passenger* p = new Passenger(name, nif);

	Reservation res(hotels[hotel - 1]->node, arrival_time, p);

	addReservation(res);
}

void UserInterface::transferMenu() {

	int i = 0;

	//distributing the passengers through  the available vans
	assignClientsToVans();

	for (unsigned int i = 0; i < vans.size(); i++) {

		vector<Node * > total_path;

		Node *currNode = source;
		Node *nextNode;

		if (vans[i]->passengers.empty())
			continue;

		while (!vans[i]->passengers.empty()) {
			vector<Node * > semi_path;
			Reservation nextPassenger = currNode->getClosestDestination(
					vans[i]->passengers);
			nextNode = nextPassenger.getDestination();

			if (nextNode != currNode){
				semi_path = transferTo(currNode->getId(), nextNode->getId());
				total_path.insert(total_path.end(), semi_path.begin(), semi_path.end());
				semi_path.clear();
			}

			cout << "Van " << i << " transfered client "
					<< nextPassenger.getClient()->getName() << endl;

			for (unsigned int j = 0; j < vans[i]->passengers.size(); j++) {
				if (vans[i]->passengers[j].getClient()->getName()
						== nextPassenger.getClient()->getName()) {
					vans[i]->passengers.erase(vans[i]->passengers.begin() + j);
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

	//puts the 10 first arrivals in the transfer
	/**cout << "Passengers to be transferred:\n";
	 while(!reservations.empty() && i < MAX_PASSENGERS){
	 reserv_vec.push_back(reservations.top().getDestination());
	 cout << reservations.top().getClient()->getName()<<endl;
	 reservations.pop();
	 i++;
	 }


	 while(!reserv_vec.empty()){
	 nextNode = currNode->getClosestNode(reserv_vec);
	 cout<<"next: "<<nextNode->getId()<<endl<<reserv_vec.size()<<endl;cin.get();
	 if(nextNode != currNode){
	 transferTo(currNode->getId(),nextNode->getId());
	 cout << "Client transfered!\n";
	 }

	 else{
	 cout << "Client transfered!\n";
	 }

	 currNode = nextNode;


	 for(i = 0; i < reserv_vec.size(); i++){
	 if(reserv_vec[i]->getId() == currNode->getId()){
	 reserv_vec.erase(reserv_vec.begin() + i);
	 break;
	 }
	 }
	 }
	 */

	return;
}

void UserInterface::mainMenu() {

	cout<<"Welcome!\n";
	cout<<"Please choose an option: \n";
	cout<<"1 - Add Reservation\n";
	cout<<"2 - Show Reservations\n";
	cout<<"3 - Plan Trip\n";
	cout<<"4 - Show Map\n";
	cout<<"5 - Exit\n";
	cout<<"\nYour option: ";

	int op;
	cin >> op;
	cin.ignore(1000, '\n');

	while(op<1 || op>5)
		cout<<"Please choose a valid option: ";
	vector<Node* > empty;

	switch (op) {
	case 1:
		cout << "\n\n\n";
		reservationMenu();
		cout << "\n\n\n";
		mainMenu();
		break;
	case 2:
		cout << "\n\n\n";
		printReservations();
		cout << "\n\n\n";
		mainMenu();
		break;
	case 3:
		cout << "\n\n\n";
		updateCoordinates();
		transferMenu();
		cout<<"\n\n\n";
		mainMenu();
		break;
	case 4:
		cout<<"\n\n\n";
		updateCoordinates();
		cout << minLat << " " << maxLat << " " << minLng << " " << maxLng << endl;

		displayGraph(empty);
		getchar();
		cout<<"\n\n\n";
		mainMenu();
		break;
	case 5:
		cout<<"\nGoodBye!\n";
		exit(0);
	}
	cin.get();
}

void UserInterface::displayGraph(vector<Node *> path)
{
	GraphViewer *gv = new GraphViewer(600, 600, false);
	//gv->setBackground("map2.png");

	gv->createWindow(1200, 1200);
	gv->defineEdgeColor("blue");
	gv->defineVertexColor("yellow");




	for(unsigned int i = 0; i < graph->vertexSet.size(); i++)
	{
		double lat = graph->vertexSet[i]->getCoordinates().getLatitude();
		double lng = graph->vertexSet[i]->getCoordinates().getLongitude();
		int x, y;

		x = floor(((lng-minLng)*4200/(maxLng-minLng)));
		y = 3183-floor(((lat-minLat)*3183/(maxLat-minLat)));

		gv->addNode(graph->vertexSet[i]->getId(), x, y);
		gv->setVertexLabel(graph->vertexSet[i]->getId(), ".");

	}

	int k = 1;
	for(unsigned int i = 0; i < graph->vertexSet.size(); i++)
	{
		for(unsigned int j = 0; j < graph->vertexSet[i]->adj.size(); j++)
		{
			gv->addEdge(graph->vertexSet[i]->adj[j]->getEdgeID(), graph->vertexSet[i]->getId(), graph->vertexSet[i]->adj[j]->getDest()->getId(), EdgeType::UNDIRECTED);
			//add road names, not advised.
			//gv->setEdgeLabel(k, graph->vertexSet[i]->adj[j]->getRoad()->getName());
			k++;
		}
	}

	for(int i = 0; i < (int)(path.size()-1); i++)
	{
		gv->setVertexColor(path[i]->getId(), RED);
		for(unsigned int k = 0; k < path[i]->adj.size(); k++)
		{
			if(path[i+1]->getId() == path[i]->adj[k]->getDest()->getId())
			{
				gv->setEdgeColor(path[i]->adj[k]->getEdgeID(), RED);

			}
		}
	}


	for(unsigned int i = 0; i < hotels.size(); i++)
	{
		//gv->setVertexColor(hotels[i]->node->getId(), GREEN);
		gv->setVertexIcon(hotels[i]->node->getId(), "hotel2.png");
		gv->setVertexLabel(hotels[i]->node->getId(), hotels[i]->name);
	}

	gv->setVertexIcon(SOURCE_NODE_ID, "airport.png");

	gv->rearrange();
}

vector<Node *> UserInterface::transferTo(unsigned long id_from, unsigned long id_dest){
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

	vector<Hotel*> zone;

	unsigned int i = 0;
	while (i < NUMBER_VANS) {

		if (!hotels[indexHotel]->getAssigned()) { //finds the first unassigned hotel
			i++;
			hotels[indexHotel]->setAssigned(true);
			zone.push_back(hotels[indexHotel]);

			for (unsigned int j = 1; j < hotelsPerVan; j++) { //fills the vector with the closest hotels

				for (unsigned int k = 0; k < hotels.size(); k++) {
					if (k != indexHotel && !hotels[k]->getAssigned()) {
						//zone.push_back(hotels[k]);
						dist = getDistance(
								hotels[indexHotel]->getNode()->getPoint(),
								hotels[k]->getNode()->getPoint());
						if (dist < distMin) {
							distMin = dist;
							indexDistMinMax = k;
						}
					}
				}
				hotels[indexDistMinMax]->setAssigned(true);
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

	vector<Hotel *> unassignedHotels;

	for (unsigned int m = 0; m < hotels.size(); m++) {
		if (!hotels[m]->getAssigned())
			unassignedHotels.push_back(hotels[m]);
	}

	int minVan;

	for (unsigned int m = 0; m < unassignedHotels.size(); m++) {
		distMin = INT_MAX;

		for (unsigned int n = 0; n < vans.size(); i++) {
			dist = getDistance(vans[n]->hotelZone[0]->getNode()->getPoint(),
					unassignedHotels[m]->getNode()->getPoint());

			if (dist < distMin) {
				distMin = dist;
				minVan = n;
			}
		}

		vans[minVan]->hotelZone.push_back(unassignedHotels[m]);

	}

	for (unsigned int m = 0; m < hotels.size(); m++) {
		if (!hotels[m]->getAssigned())
			unassignedHotels.push_back(hotels[m]);
	}

	cout << "There are " << unassignedHotels.size() << " unassigned hotels\n";

	for (unsigned int i = 0; i < vans.size(); i++) {
		for (unsigned int j = 0; j < vans[i]->hotelZone.size(); j++) {
			cout << "Hotel " << vans[i]->hotelZone[j]->getNode()->getId()
							<< " assigned to van " << i << endl;
		}
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
				if (vans[i]->hotelZone[j]->getNode()->getId()
						== reservations.top().getDestination()->getId()) {
					if (vans[i]->passengers.size() < MAX_PASSENGERS) {
						cout << "Passenger "
								<< reservations.top().getClient()->getName()
								<< " assigned to van " << i << endl;
						vans[i]->passengers.push_back(reservations.top());
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
}

void UserInterface::updateCoordinates()
{
	for(unsigned int i = 0; i < graph->vertexSet.size(); i++)
	{
		double lat = graph->vertexSet[i]->getCoordinates().getLatitude();
		double lng = graph->vertexSet[i]->getCoordinates().getLongitude();

		if(maxLat < lat)
			maxLat = lat;
		if(minLat > lat)
			minLat = lat;
		if(maxLng < lng)
			maxLng = lng;
		if(minLng > lng)
			minLng = lng;
	}
}

