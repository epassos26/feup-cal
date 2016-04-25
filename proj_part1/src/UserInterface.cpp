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

UserInterface::UserInterface(Graph *g, int maxPassengers){
	this->graph = g;
	this->maxPassengers = maxPassengers;
}

Hotel::Hotel(string name, int idNode)
{
	this->name = name;
	this->idNode = idNode;
	this->assigned = false;
}

bool Hotel::getAssigned(){
	return this->assigned;
}

void Hotel::setAssigned(bool flag){
	this->assigned = flag;
}

unsigned long Hotel::getIdNode(){
	return this->idNode;
}

Van::Van(vector<Hotel*> hz){
	this->hotelZone=hz;
}

void UserInterface::readHotels()
{
	ifstream file;
	file.open(HOTELS_FILENAME);


	if (!file.is_open())
		return;

	while (!file.eof()) {

		string buff;
		buff.clear();
		stringstream ss;

		string name;
		unsigned long  idNode;


		if (getline(file, buff, ';')){
			ss << buff;
			ss >> idNode;
			ss.clear();
		}

		if(!getline(file, name, ';'))
		{
			break;
		}

		Hotel* h = new Hotel(name, idNode);
		hotels.push_back(h);

	}

	gettingHotelsCloser();

	cout<<vans.size()<<endl;
	for(unsigned int i = 0 ; i<vans.size(); i++){
		cout<<vans[i]->hotelZone[0]->getIdNode()<<endl;
		cout<<vans[i]->hotelZone[1]->getIdNode()<<endl;
	}

	cin.get();
}

void UserInterface::printHotels()
{
	cout << endl;
	for(unsigned int i = 0; i < hotels.size(); i++)
	{
		cout << i+1 << " - " << hotels[i]->name << endl;
	}
}

void UserInterface::readReservations()
{
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


		if (getline(file, buff, ';')){
			ss << buff;
			ss >> nif;
			ss.clear();
		}

		if (getline(file, buff, ';')){
			ss << buff;
			ss >> hotel;
			ss.clear();
		}

		if (getline(file, buff, '\n')){
			ss << buff;
			ss >> arrival_time;
			ss.clear();
		}

		cout<<name<<" "<<nif<<" "<<hotel<<" "<<arrival_time<<endl;
		Passenger* p = new Passenger(name, nif);

		Reservation* res = new Reservation(graph->getNode(hotels[hotel-1]->idNode), arrival_time, p);

		addReservation(res);

	}
}

void UserInterface::printReservations()
{
	priority_queue<Reservation * > temp;

	cout<<"Reservations: "<<endl<<endl;
	while(!reservations.empty()){
		cout<<"Name: ";
		cout<<reservations.top()->getClient()->getName()<<endl;
		cout<<"Destination: ";
		cout<<reservations.top()->getDestination()->getId()<<endl;
		cout<<"Arrival Time: ";
		cout<<reservations.top()->getArrivalTime()<<endl<<endl;

		temp.push(reservations.top());
		reservations.pop();
	}

	while(!temp.empty()){
		reservations.push(temp.top());
		temp.pop();
	}

	cout<<"Press Enter to Continue...";
	cin.get();

}
bool UserInterface::addReservation(Reservation *r){
	priority_queue<Reservation * > temp;

	string name = r->getClient()->getName();
	while(!reservations.empty()){
		string topName = reservations.top()->getClient()->getName();
		if(topName == name){
			return false;
		}

		temp.push(reservations.top());
		reservations.pop();
	}

	while(!temp.empty()){
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

	cout<<"Reservation Menu\n";

	cout<<"Insert name: ";
	getline(cin, name);

	cout<<"Insert NIF: ";
	cin >> nif;
	cin.ignore(1000, '\n');

	printHotels();
	cout << "Choose Destination: ";
	cin >> hotel;
	cin.ignore(1000, '\n');
	while(hotel<1 || hotel>hotels.size()){
		cout<<"Please choose a valid hotel: ";
		cin >> hotel;
		cin.ignore(1000, '\n');
	}

	cout<<"Insert Arrival Time: ";
	cin >> arrival_time;
	cin.ignore(1000, '\n');

	Passenger* p = new Passenger(name, nif);

	Reservation* res = new Reservation(graph->getNode(hotels[hotel-1]->idNode), arrival_time, p);

	addReservation(res);
}

void UserInterface::mainMenu() {
	cout<<"Welcome!\n";
	cout<<"Please choose an option: \n";
	cout<<"1 - Add Reservation\n";
	cout<<"2 - Show Reservations\n";
	cout<<"3 - Plan Trip\n";
	cout<<"4 - Exit\n";
	cout<<"\nYour option: ";

	int op;
	cin>>op;
	cin.ignore(1000, '\n');

	while(op<1 || op>4)
		cout<<"Please choose a valid option: ";

	switch(op){
	case 1:
		cout<<"\n\n\n";
		reservationMenu();
		cout<<"\n\n\n";
		mainMenu();
		break;
	case 2:
		cout<<"\n\n\n";
		printReservations();
		cout<<"\n\n\n";
		mainMenu();
		break;
	case 3:
		//TODO plan the trip for the clients who arrive first
		break;
	case 4:
		cout<<"\nGoodBye!\n";
		exit(0);
	}
	cin.get();
}

void UserInterface::goTo(unsigned long id_dest){

	//unsigned long id_origin = 3713666414; //3713666414 -> 25532201
	unsigned long id_origin = 112617168; //112617168 -> 126604785
	vector<Node *> path;


	graph->resetIndegrees();
	//graph->bellmanFordShortestPath(id_dest);
	//cout<<"Dijkstra\n"; cin.get();//TODO remove
	graph->dijkstraShortestPath(id_origin);
	//cout<<"Dijkstra over\n";cin.get();

	path = graph->getPath(id_origin, id_dest);

	string oldRoadName = "1";

	for(unsigned int i=0; i<path.size()-1; i++){
		for(unsigned int j=0; j < path[i]->adj.size(); j++){
			if(path[i]->adj[j]->getDest()->getId() == path[i+1]->getId()){

				if(oldRoadName != path[i]->adj[j]->getRoad()->getName()){
					oldRoadName = path[i]->adj[j]->getRoad()->getName();
					cout<<"Turn to road ";
					if(oldRoadName.size() == 0)
						cout<<"Unknown Name"<<endl;
					else
						cout<<oldRoadName<<endl;

				}
			}
		}
		cout<<" Passing Node "<<path[i]->getId()<<endl;
	}

	cout<<"You are on destiny!"<<endl;
	cout<<"Total Distance : "<<path[path.size()-1]->dist<<endl;
}

void UserInterface::gettingHotelsCloser(){

	unsigned int hotelsPerVan = hotels.size()/NUMBER_VANS;
	unsigned int indexHotel = 0;
	int distMin = INT_MAX;
	int indexDistMinMax;
	int dist;

	vector<Hotel*> zone;

	unsigned int i=0;
	while(i < NUMBER_VANS){

		if(!hotels[indexHotel]->getAssigned()){
			i++;
			hotels[indexHotel]->setAssigned(true);
			zone.push_back(hotels[indexHotel]);

			for(unsigned int j=1; j<hotelsPerVan; j++){

				for(unsigned int k=0; k<hotels.size();k++){
					if( k != indexHotel && !hotels[k]->getAssigned()){
						zone.push_back(hotels[k]);
						dist = getDistance(graph->getNode(hotels[indexHotel]->getIdNode())->getPoint(), graph->getNode(hotels[k]->getIdNode())->getPoint());
						if(dist < distMin){
							distMin = dist;
							indexDistMinMax = k;
						}
					}
				}
				hotels[indexDistMinMax]->setAssigned(true);
				zone.push_back(hotels[indexDistMinMax]);
				dist = INT_MAX;
			}
			indexHotel++;
			vans.push_back(new Van(zone));
			zone.clear();
		}
		else if(indexHotel < hotels.size()-1){
			indexHotel++;
		}
		else indexHotel = 0;
	}
}



