/*** This is just a Skeleton/Starter Code for the External Storage Assignment. This is by no means absolute, in terms of assignment approach/ used functions, etc. ***/
/*** You may modify any part of the code, as long as you stick to the assignments requirements we do not have any issue ***/

// Include necessary standard library headers
#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
#include <fstream>
using namespace std; // Include the standard namespace

class Record {
public:
    int id, manager_id; // Employee ID and their manager's ID
    std::string bio, name; // Fixed length string to store employee name and biography

    Record(vector<std::string> &fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    //You may use this for debugging / showing the record to standard output. 
    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }

    // Function to get the size of the record
    int get_size() {
        // sizeof(int) is for name/bio size() in serialize function
        return sizeof(id) + sizeof(manager_id) + sizeof(int) + name.size() + sizeof(int) + bio.size(); 
    }
    
    // Take a look at Figure 9.9 and read the Section 9.7.2 [Record Organization for Variable Length Records]
    // TODO: Consider using a delimiter in the serialize function to separate these items for easier parsing.
    string serialize() const {
        ostringstream oss;
        oss.write(reinterpret_cast<const char*>(&id), sizeof(id)); // Writes the binary representation of the ID.
        oss.write(reinterpret_cast<const char*>(&manager_id), sizeof(manager_id)); // Writes the binary representation of the Manager id
        int name_len = name.size();
        int bio_len = bio.size();
        oss.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len)); // // Writes the size of the Name in binary format.
        oss.write(name.c_str(), name.size()); // writes the name in binary form
        oss.write(reinterpret_cast<const char*>(&bio_len), sizeof(bio_len)); // // Writes the size of the Bio in binary format. 
        oss.write(bio.c_str(), bio.size()); // writes bio in binary form

        // Add a delimiter after each record for easier parsing
        char delimiter = '\0'; // Null character as the delimiter
        oss.write(&delimiter, sizeof(delimiter)); // Write the delimiter

        // Return the serialized string
        return oss.str();
    }
};

class page{ // Take a look at Figure 9.7 and read Section 9.6.2 [Page organization for variable length records] 
public:
    vector <Record> records; // Data Area: Stores records. 
    vector <pair <int, int>> slot_directory; // This slot directory contains the starting position (offset), and size of the record. 
                                        
    int cur_size = 0; // holds the current size of the page

    // Function to insert a record into the page
    bool insert_record_into_page(Record r) {
        int record_size = r.get_size();
        int slot_size = sizeof(int) * 2;
        if (cur_size + record_size + slot_size > 4096) { //Check if page size limit exceeded, considering slot directory size
            return false; // Cannot insert the record into this page
        } else {
            int offset = cur_size; // The starting position (offset) for the new record
            records.push_back(r); // Record stored in current page
            cur_size += r.get_size(); // Updating page size

            // TO_DO: update slot directory information
            
            slot_directory.emplace_back(offset, record_size); // Add a new entry to the slot directory

            return true;
        }
        

    }

    // Function to write the page to a binary file, i.e., EmployeeRelation.dat file
    void write_into_data_file(ostream& out) const { 
        
        char page_data[4096] = {0}; // Write the page contents (records and slot directory) into this char array so that the page can be written to the data file in one go.

        int offset = 0; // Used as an iterator to indicate where the next item should be stored. Section 9.6.2 contains information that will help you with the implementation.

        for (const auto& record : records) { // Writing the records into the page_data
            string serialized = record.serialize();

            memcpy(page_data + offset, serialized.c_str(), serialized.size());

            offset += serialized.size();
        }

        // TO_DO: Put a delimiter here to indicate slot directory starts from here 

        page_data[offset++] = '\0'; // Add a null character as the delimiter

        for (const auto& slots : slot_directory) { 
            // TO_DO: Write the slot-directory information into page_data. You'll use slot-directory to retrieve record(s).
            if (slots.first < 0 || slots.first >= 4096 || slots.second <= 0 || slots.second > 4096) {
                cerr << "Error: Invalid slot being written! Offset: " << slots.first << ", Size: " << slots.second << endl;
                continue; // skip the error slot to prevent invalid data from being written 
            }
            cout << "Writing slot - Offset: " << slots.first << ", Size: " << slots.second << endl; // Debug
            memcpy(page_data + offset, &slots.first, sizeof(int)); // Write offset
            offset += sizeof(int);
            memcpy(page_data + offset, &slots.second, sizeof(int)); // Write size
            offset += sizeof(int);

        }
        
        out.write(page_data, sizeof(page_data)); // Write the page_data to the EmployeeRelation.dat file 

    }

    // Read a page from a binary input stream, i.e., EmployeeRelation.dat file to populate a page object
    bool read_from_data_file(istream& in) {
        cout << "read_from_data_file() called!" << endl;
        char page_data[4096] = {0}; // Character array used to read 4 KB from the data file to your main memory. 
        in.read(page_data, 4096); // Read a page of 4 KB from the data file 


        streamsize bytes_read = in.gcount(); // used to check if 4KB was actually read from the data file
        if (bytes_read == 4096) {
            
            // TO_DO: You may process page_data (4 KB page) and put the information to the records and slot_directory (main memory).
            // TO_DO: You may modify this function to process the search for employee ID in the page you just loaded to main memory.
            int offset = 0;
            records.clear(); // Clear existing records
            // Debugging: Print the initial offset
            cout << "Initial offset: " << offset << endl;

            while (offset < 4096 - sizeof(int) * 2) {  // Stop at the delimiter
                // Deserialize the record manually using the `Record` constructor
                int id, manager_id, name_len, bio_len;

                memcpy(&id, page_data + offset, sizeof(int)); // Read ID
                offset += sizeof(int);

                memcpy(&manager_id, page_data + offset, sizeof(int)); // Read Manager ID
                offset += sizeof(int);

                // if (offset + name_len >= 4096) {
                //     cerr << "Error: Name field out of bounds!" << endl;
                //     return false;
                // }
                memcpy(&name_len, page_data + offset, sizeof(int)); // Read Name length
                offset += sizeof(int);

                string name(page_data + offset, name_len); // Read Name
                offset += name_len;

                memcpy(&bio_len, page_data + offset, sizeof(int)); // Read Bio length
                offset += sizeof(int);

                string bio(page_data + offset, bio_len); // Read Bio
                offset += bio_len;

                // Construct Record object and add it to records
                vector<string> fields = {to_string(id), name, bio, to_string(manager_id)};
                Record record(fields);
                records.push_back(record);

                // Debugging: Print the offset after reading each record
                cout << "Record read, current offset: " << offset << endl;
            }
            // Skip the delimiter
            offset++;

            // Debugging: Print the offset after skipping the delimiter
            cout << "Offset after skipping delimiter: " << offset << endl;

            // TO_DO: Populate the slot directory
            slot_directory.clear(); // Clear existing slot directory
            while (offset + sizeof(int) * 2 <= 4096) { // Ensure there's enough space for offset and size
                int record_offset, record_size;
                cout<<"Infinite for loop"<<endl;
                memcpy(&record_offset, page_data + offset, sizeof(int)); // Read offset
                offset += sizeof(int);

                memcpy(&record_size, page_data + offset, sizeof(int)); // Read size
                offset += sizeof(int);
                cout << "Parsed Slot - Offset: " << record_offset << ", Size: " << record_size << endl;
                if (record_offset < 0 || record_offset >= 4096 || record_size <= 0 || record_size > 4096) {
                    // cout << "Invalid slot detected, breaking loop!" << endl;
                    cerr << "Invalid slot detected in read! Offset: " << record_offset << ", Size: " << record_size << ". Breaking loop!" << endl;
                    break;
                }
                slot_directory.emplace_back(record_offset, record_size); // Add entry to the slot directory
                // Debugging: Print the offset after reading each slot
                cout << "Slot read, current offset: " << offset << endl;
            }
            // Debugging: Print the final offset
            cout << "Final offset: " << offset << endl;
            return true;
        }

        if (bytes_read > 0) { 
            cerr << "Incomplete read: Expected " << 4096 << " bytes, but only read " << bytes_read << " bytes." << endl;
        }

        return false;
    }
};

class StorageManager {

public:
    string filename;  // Name of the file (EmployeeRelation.dat) where we will store the Pages 
    fstream data_file; // fstream to handle both input and output binary file operations
    vector <page> buffer; // You can have maximum of 3 Pages.
    
    // Constructor that opens a data file for binary input/output; truncates any existing data file
    StorageManager(const string& filename) : filename(filename) {
        data_file.open(filename, ios::binary | ios::out | ios::in | ios::trunc);
        if (!data_file.is_open()) {  // Check if the data_file was successfully opened
            cerr << "Failed to open data_file: " << filename << endl;
            exit(EXIT_FAILURE);  // Exit if the data_file cannot be opened
        }
    }

    // Destructor closes the data file if it is still open
    ~StorageManager() {
        if (data_file.is_open()) {
            data_file.close();
        }
    }

    // Reads data from a CSV file and writes it to EmployeeRelation.dat
    void createFromFile(const string& csvFilename) {
        buffer.resize(3); // You can have maximum of 3 Pages.

        ifstream csvFile(csvFilename);  // Open the Employee.csv file for reading
        
        string line, name, bio;
        int id, manager_id;
        int page_number = 0; // Current page we are working on [at most 3 pages]

        while (getline(csvFile, line)) {   // Read each line from the CSV file, parse it, and create Employee objects
            stringstream ss(line);
            string item;
            vector<string> fields;

            while (getline(ss, item, ',')) {
                fields.push_back(item);
            }
            Record r = Record(fields);  //create a record object            

            
            if (!buffer[page_number].insert_record_into_page(r)) { // inserting that record object to the current page
                
                // Current page is full, move to the next page
                page_number++;
 
                if (page_number >= buffer.size()) {    // Checking if page limit has been reached.
                    
                    for (page& p : buffer) { // using write_into_data_file() to write the pages into the data file
                        p.write_into_data_file(data_file);
                    }
                    page_number = 0; // Starting again from page 0

                }
                buffer[page_number].insert_record_into_page(r); // Reattempting the insertion of record 'r' into the newly created page
            }
            
        }
        csvFile.close();  // Close the CSV file
    }

    // Searches for an Employee ID in EmployeeRelation.dat
    void findAndPrintEmployee(int searchId) {
        
        data_file.seekg(0, ios::beg);  // Rewind the data_file to the beginning for reading

        // TO_DO: Read pages from your data file (using read_from_data_file) and search for the employee ID in those pages. Be mindful of the page limit in main memory.        
        int page_number = 0;
        while(buffer[page_number].read_from_data_file(data_file)){
            cout << "Checking Page: " << page_number << endl;
           for(Record& r : buffer[page_number].records) {
            cout << "Checking Employee ID: " << r.id << endl;
                if(r.id ==searchId ) {
                    cout<<"We have found the employee with id "<< r.id<< "and name"<<r.name<<endl;;
                    return;
                }
           }

            page_number++;
        }

        data_file.clear();
        cout<<"We can't find the employee with that id "<<endl;

    }
};
