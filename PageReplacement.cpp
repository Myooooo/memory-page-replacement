#include <fstream>
#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <algorithm>

using namespace std;

// data structs
struct Event
{
    bool op;    // false for read, true for write
    int addr;   // memory address
};

struct Frame
{
    int page;   // loaded page of the frame
    bool dirty; // marks if the page is dirty
    int age;    // marks how long the page has been unused
    string R;   // reference bits
    int C;   // reference frequency

    Frame(int p)
    {
        page = p;
        dirty = false;
        age = 0;
        R = "0";
        C = 0;
    }

    Frame(int p, bool d)
    {
        page = p;
        dirty = d;
        age = 0;
        R = "0";
        C = 0;
    }

    Frame(int p, bool d, string r)
    {
        page = p;
        dirty = d;
        age = 0;
        R = r;
        C = 0;
    }
};

// global variables
queue<Event> traces;
vector<Frame*> frames;
vector<Frame*> workingSet;
int events = 0;
int reads = 0;
int writes = 0;
int faults = 0;

// reads input from path
void input(string path)
{
    ifstream inputFile(path);
    string line;

    while(getline(inputFile, line))
    {
        bool op;
        int addr;

        if(line.at(0) == 'R')
        {
            op = false;
            addr = stoi(line.substr(2),nullptr,16);
            traces.push({op,addr});
            events++;
        }
        else if(line.at(0) == 'W')
        {
            op = true;
            addr = stoi(line.substr(2),nullptr,16);
            traces.push({op,addr});
            events++;
        }
    }

    inputFile.close();
}

// returns the index of the input page number in the loaded frames
// if page not loaded, return -1
int getPageIndex(int page)
{
    int index = -1;

    for(int i = 0; i < frames.size(); i++)
    {
        if(page == frames[i]->page)
        {
            index = i;
            break;
        }
    }
    
    return index;
}

// returns the index of the input page number in the working set
// if page not active, return -1
int getWSIndex(Frame* frame)
{
    int index = -1;

    for(int i = 0; i < workingSet.size(); i++)
    {
        if(frame->page == workingSet[i]->page)
        {
            index = i;
            break;
        }
    }

    return index;
}

// update the reference frequencies from the working set
void updateWorkingSet(Frame* frame, int delta)
{
    workingSet.push_back(frame);
    if(workingSet.size() > delta)
    {
        Frame* temp = workingSet[0];
        workingSet.erase(workingSet.begin());
        if(getWSIndex(temp) == -1) temp->C = 0;
    }

    for(int i = 0; i < workingSet.size(); i++)
    {
        int n = count(workingSet.begin(),workingSet.end(),workingSet[i]);
        workingSet[i]->C = n;
    }
}

// compare functions

bool sortARB(Frame* a, Frame* b)
{
    if(a->page == -1 || b->page == -1)
    {
        return a->page < b->page;
    }

    if(a->R != b->R)
    {
        return stoi(a->R) < stoi(b->R);
    }
    else
    {
        return a->age > b->age;
    }
}

bool sortWSARB1(Frame* a, Frame* b)
{
    if(a->page == -1 || b->page == -1)
    {
        return a->page < b->page;
    }

    if(a->C != b->C)
    {
        return a->C < b->C;
    }
    
    if(a->R != b->R)
    {
        return stoi(a->R) < stoi(b->R);
    }
    else
    {
        return a->age > b->age;
    }
}

bool sortWSARB2(Frame* a, Frame* b)
{
    if(a->page == -1 || b->page == -1)
    {
        return a->page < b->page;
    }

    if(a->R != b->R)
    {
        return stoi(a->R) < stoi(b->R);
    }

    if(a->C != b->C)
    {
        return a->C < b->C;
    }
    else
    {
        return a->age > b->age;
    }
}

// process functions

void processFIFO(int pageSize)
{
    int counter = 1;

    while(!traces.empty())
    {
        Event event = traces.front();
        traces.pop();
        
        int page = event.addr / pageSize;
        bool dirty = false;
        int index = getPageIndex(page);

        if(index == -1)
        {
            // page not already loaded

            //printf("Time: %d \t MISS: \t%d", counter, page);

            Frame* replaced = frames[0];
            frames.erase(frames.begin());

            //printf("\tREPLACE: page %d ",replaced->page);
            
            if(replaced->dirty)
            {
                //printf("(DIRTY)");
                writes++;
            }

            if(event.op) dirty = true;

            Frame* frame = new Frame(page,dirty);
            frames.push_back(frame);
            reads++;

            delete replaced;
            
            //printf("\n");
        }
        else
        {
            // page already loaded
            //printf("Time: %d \t HIT: \t%d\n", counter, page);
            if(event.op) frames[index]->dirty = true;
        }
        
        counter++;
    }
}

void processLRU(int pageSize)
{
    int counter = 1;

    while(!traces.empty())
    {
        Event event = traces.front();
        traces.pop();
        
        int page = event.addr / pageSize;
        bool dirty = false;
        int index = getPageIndex(page);

        if(index == -1)
        {
            // page not already loaded

            //printf("Time: %d \t MISS: \t%d", counter, page);

            // sort frames according to descending age
            sort(frames.begin(), frames.end(), [](const Frame* a, const Frame* b)
			    {return  a->age > b->age;});

            Frame* replaced = frames[0];
            frames.erase(frames.begin());

            //printf("\tREPLACE: page %d ",replaced->page);
            
            if(replaced->dirty)
            {
                //printf("(DIRTY)");
                writes++;
            }

            if(event.op) dirty = true;

            // aging unused pages
            for(int i = 0; i < frames.size(); i++) frames[i]->age++;

            Frame* frame = new Frame(page,dirty);
            frames.push_back(frame);
            reads++;

            delete replaced;
            
            //printf("\n");
        }
        else
        {
            // page already loaded

            //printf("Time: %d \t HIT: \t%d\n", counter, page);
            if(event.op) frames[index]->dirty = true;

            // aging unused pages
            for(int i = 0; i < frames.size(); i++) frames[i]->age++;

            frames[index]->age = 0; 
        }
        
        counter++;
    }
}

void processARB(int pageSize, int a, int b)
{
    int counter = 1;

    // generate initial reference bits
    string R = "1";
    for(int i = 1; i < a; i++) R += "0";

    while(!traces.empty())
    {
        Event event = traces.front();
        traces.pop();
        
        int page = event.addr / pageSize;
        bool dirty = false;
        int index = getPageIndex(page);

        if(index == -1)
        {
            // page not already loaded

            //printf("Time: %d \t MISS: \t%d", counter, page);

            // sort frames according to ascending reference bits
            sort(frames.begin(), frames.end(), sortARB);
            
            Frame* replaced = frames[0];
            frames.erase(frames.begin());
            
            //printf("\tREPLACE: page %d ",replaced->page);
            
            if(replaced->dirty)
            {
                //printf("(DIRTY)");
                writes++;
            }

            if(event.op) dirty = true;

            Frame* frame = new Frame(page,dirty,R);
            frames.push_back(frame);
            reads++;

            delete replaced;
            
            //printf("\n");
        }
        else
        {
            // page already loaded
            //printf("Time: %d \t HIT: \t%d\n", counter, page);
            if(event.op) frames[index]->dirty = true;
            frames[index]->R[0] = '1';
        }

        /*cout << "\tframes: ";
        for(int i = 0; i < frames.size(); i++)
        {
            cout << frames[i]->page << "(" << frames[i]->R << ")(" << frames[i]->age << ")  ";
        }
        printf("\n");*/

        // shift ARBs to the right every interval
        if((counter % b) == 0)
        {
            for(int i = 0; i < frames.size(); i++)
            {
                string temp = frames[i]->R;
                temp.assign(temp.begin(),temp.end() - 1);
                temp = "0" + temp;
                frames[i]->R = temp;
            }
        }

        // aging pages every cycle to record arriving order
        for(int i = 0; i < frames.size(); i++) frames[i]->age++;
        
        counter++;
    }
}

void processWSARB1(int pageSize, int a, int b, int delta)
{
    int counter = 1;

    // generate initial reference bits
    string R = "1";
    for(int i = 1; i < a; i++) R += "0";

    while(!traces.empty())
    {
        Event event = traces.front();
        traces.pop();
        
        int page = event.addr / pageSize;
        bool dirty = false;
        int index = getPageIndex(page);

        if(index == -1)
        {
            // page not already loaded

            //printf("Time: %d \t MISS: \t%d", counter, page);

            // sort frames according to ascending reference frequency
            sort(frames.begin(), frames.end(), sortWSARB1);
            
            Frame* replaced = frames[0];
            frames.erase(frames.begin());
            
            //printf("\tREPLACE: page %d ",replaced->page);
            
            if(replaced->dirty)
            {
                //printf("(DIRTY)");
                writes++;
            }

            if(event.op) dirty = true;
            
            Frame* frame = new Frame(page,dirty,R);
            frames.push_back(frame);
            updateWorkingSet(frame,delta);
            reads++;

            if(getWSIndex(replaced) == -1) delete replaced;

            //printf("\n");
        }
        else
        {
            // page already loaded
            //printf("Time: %d \t HIT: \t%d\n", counter, page);
            if(event.op) frames[index]->dirty = true;
            frames[index]->R[0] = '1';
            updateWorkingSet(frames[index],delta);
        }

        /*cout << "\tframes: ";
        for(int i = 0; i < frames.size(); i++)
        {
            cout << frames[i]->page << "(" << frames[i]->R << ")(" << frames[i]->age << ")(" << frames[i]->C << ")  ";
        }
        printf("\n");

        cout << "\tworking set: ";
        for(int i = 0; i < workingSet.size(); i++)
        {
            cout << workingSet[i]->page << "(" << workingSet[i]->C << ")  ";
        }
        printf("<%ld pages>\n", workingSet.size());*/

        // shift ARBs to the right every interval
        if((counter % b) == 0)
        {
            for(int i = 0; i < frames.size(); i++)
            {
                string temp = frames[i]->R;
                temp.assign(temp.begin(),temp.end() - 1);
                temp = "0" + temp;
                frames[i]->R = temp;
            }
        }

        // aging pages every cycle to record arriving order
        for(int i = 0; i < frames.size(); i++) frames[i]->age++;
        
        counter++;
    }
}

void processWSARB2(int pageSize, int a, int b, int delta)
{
    int counter = 1;

    // generate initial reference bits
    string R = "1";
    for(int i = 1; i < a; i++) R += "0";

    while(!traces.empty())
    {
        Event event = traces.front();
        traces.pop();
        
        int page = event.addr / pageSize;
        bool dirty = false;
        int index = getPageIndex(page);

        if(index == -1)
        {
            // page not already loaded

            //printf("Time: %d \t MISS: \t%d", counter, page);

            // sort frames according to ascending reference frequency
            sort(frames.begin(), frames.end(), sortWSARB2);
            
            Frame* replaced = frames[0];
            frames.erase(frames.begin());
            
            //printf("\tREPLACE: page %d ",replaced->page);
            
            if(replaced->dirty)
            {
                //printf("(DIRTY)");
                writes++;
            }

            if(event.op) dirty = true;
            
            Frame* frame = new Frame(page,dirty,R);
            frames.push_back(frame);
            updateWorkingSet(frame,delta);
            reads++;

            if(getWSIndex(replaced) == -1) delete replaced;

            //printf("\n");
        }
        else
        {
            // page already loaded
            //printf("Time: %d \t HIT: \t%d\n", counter, page);
            if(event.op) frames[index]->dirty = true;
            frames[index]->R[0] = '1';
            updateWorkingSet(frames[index],delta);
        }

        /*cout << "\tframes: ";
        for(int i = 0; i < frames.size(); i++)
        {
            cout << frames[i]->page << "(" << frames[i]->R << ")(" << frames[i]->age << ")(" << frames[i]->C << ")  ";
        }
        printf("\n");

        cout << "\tworking set: ";
        for(int i = 0; i < workingSet.size(); i++)
        {
            cout << workingSet[i]->page << "(" << workingSet[i]->C << ")  ";
        }
        printf("<%ld pages>\n", workingSet.size());*/

        // shift ARBs to the right every interval
        if((counter % b) == 0)
        {
            for(int i = 0; i < frames.size(); i++)
            {
                string temp = frames[i]->R;
                temp.assign(temp.begin(),temp.end() - 1);
                temp = "0" + temp;
                frames[i]->R = temp;
            }
        }

        // aging pages every cycle to record arriving order
        for(int i = 0; i < frames.size(); i++) frames[i]->age++;
        
        counter++;
    }
}

void process(int pageSize, int frameNo, string alg, int a, int b, int delta)
{
    for(int i = 0; i < frameNo; i++)
    {
        Frame* frame = new Frame(-1);
        frames.push_back(frame);
    }

    if(alg == "FIFO")
    {
        processFIFO(pageSize);
    }
    else if(alg == "LRU")
    {
        processLRU(pageSize);
    }
    else if(alg == "ARB")
    {
        processARB(pageSize, a, b);
    }
    else if(alg == "WSARB-1")
    {
        processWSARB1(pageSize, a, b, delta);
    }
    else if(alg == "WSARB-2")
    {
        processWSARB2(pageSize, a, b, delta);
    }

    faults = reads;
}

void output()
{
    cout << "events in trace: " << events << endl;
    cout << "total disk reads: " << reads << endl;
    cout << "total disk writes: " << writes << endl;
    cout << "page faults: " << faults << endl;
}

void cleanup()
{
    for(int i = 0; i < frames.size(); i++)
    {
        delete frames[i];
    }
}

// main function
int main(int argc,char** argv)
{
    string path(argv[1]);
    int pageSize = atoi(argv[2]);
    int frameNo = atoi(argv[3]);
    string alg(argv[4]);
    int a = 0;
    int b = 0;
    int delta = 0;

    if(alg == "ARB" || alg == "WSARB-1" || alg == "WSARB-2")
    {
        a = atoi(argv[5]);
        b = atoi(argv[6]);
        if(alg != "ARB") delta = atoi(argv[7]);
    }

    input(path);
    process(pageSize,frameNo,alg,a,b,delta);
    output();
    cleanup();

    return 0;
}