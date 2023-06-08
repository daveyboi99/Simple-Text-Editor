#ifndef MVC_H
#define MVC_H
#include "ConcreteObserver.h"
#include "Command.h"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
//create an MVC 


//****************************************************************************
//we need to create then model here



//model to describe layout
//model updates the view which is TextViewImp
//concrete command object controll the 

class Model
{

public:
    
    Model(std::string file) : file_name(file){}
    ~Model(){ text.clear(); }

    //get the number of rows
    int GetNumRows() const {return NumRows;}
    //get the size of the text at the current row
    int GetTextSize() const{return text.size(); }

    std::string GetFileName() const { return file_name; }

    int GetCurrRowSize(int y) const {return text[y].size();}
    
    void ReadFile()
    {
        std::string str;
        std::ifstream file;
        file.open(file_name);
        if(file.is_open())
        {
            while(std::getline(file, str))
            {
                text.emplace_back(str);
            }
            if (text.empty()) text.push_back("");
            file.close();
        }

    }

    //get the char from the current position
    char GetCharAt(int x, int y) const
    {
        if(x < text[y].size() && y < text.size())
        {
            return (char)text[y][x];
        }

    }


    //get a specific row
    std::string GetRow(int y)
    {
        return text[y];
    }

    //return the text of the current row        
    std::vector<std::string> GetText () const
    {
        return text;
    }

    
    void AddCharAt(const char c, int x, int y, int bound)
    {
        if(text.size() <= y) 
        {
            std::string temp = "";
            temp.push_back(c);
            text.push_back(temp);
        }
        //was else if (x < bound)
        else {
            text[y].insert(text[y].begin() + x, c);
        }
        //text wrap if x is at the row limit
    }


    //method to add an empty row at a specified y position
    //can be used for text wrapping as well
    void AddRowAt(int y)
    {
        //if the row to be added doesnt exist, then just push back an empty row and add to that

        std::vector<std::string> temp = text;
        text.push_back("");
        for(int i = y; i < text.size()-1; i++)
        {
            text[i+1] = temp[i];
        }
        text[y] = "";

        //else add to the current row
    }

    void RemoveCharAt(int x, int y, int bound)
    {

        //case where we we delete from a row
        if(x >= text[y].size()  && x != 0 || (text[y].size() >= x && x != 0))
        {
            //
            text[y].erase(text[y].begin() + x-1);
            //text[y].pop_back();
        }
        
        //merge current row with the row before if x is 0
        else if(x == 0 && text[y-1].size() < bound && y > 0)
        {
            //do something
            std::string temp = text[y];
            text[y-1] += temp;
            text[y].erase(text[y].begin(), text[y].end());

            std::vector<std::string> str = text;


            for(int i = text.size()-1; i > y; i--)
            {             
                //was += befor
                text[i-1] = str[i];
            }
            text.pop_back();            
        }
        
    }



    void RemoveRow(int y)
    {
        std::vector<std::string> temp =text;
        //pop back the list
        text[y].erase(text[y].begin(), text[y].end());
        for(int i = text.size()-1; i > y; i--)
        {
            text[i-1] = temp[i];
        }
        

        text.pop_back();

    }
    
    //methods to add to the view
    void AddText(const std::string r)
    {
        //do something
        text.push_back(r);
    }

    //method to add the text at a certain place
    void AddTextAt(const std::string str, int y)
    {
        text[y] = str;
    }

    void SetText(const std::vector<std::string> &temp)
    {
        text.clear();
        text = temp;
    }

    void RemoveText(int pos)
    {
        text[pos].pop_back();
    }

    //update the view when the curor moves


private:
    //keep track of the rows to add to the view
    std::vector<std::string> text;
    //hold an instance of the view

    //get the name of the file
    std::string file_name;

    //store the current number of rows
    int NumRows;

};
//******************************************************************************************************************

//concrete command classes  



class AddTextAt: public Command
{
public:
    
    AddTextAt(Model &m, char ch, int x, int y, int b): model(m), c(ch), pX(x), pY(y), bound(b){}
    ~AddTextAt(){}

    void Execute()
    {
        //wait until cursor is done
        model.AddCharAt(c, pX, pY, bound);
        
    }
    void UnExecute()
    {
        //basically undo the command
        model.RemoveCharAt(pX+1, pY, bound);
    }


private:
    //have to keep an instance of the view
    Model &model;
    //char to add to the view
    char c;
    int pX;
    int pY;
    int bound;
};




class RemoveTextAt: public Command
{
public:
    RemoveTextAt(Model &m, int x, int y, int b): model(m), pX(x), pY(y), bound(b){}
    ~RemoveTextAt(){}

    void Execute()
    {
        //if x is is zero, add the current row to the removed chars
        if(pX == 0 && pY != 0)
        {   old_len = model.GetRow(pY-1).size();
            std::string temp = model.GetRow(pY);
            for(auto it: temp) removed_chars.push_back(it);
        }
        else removed_chars.push_back(model.GetCharAt(pX-1,pY));

        model.RemoveCharAt(pX, pY, bound);
    }

    void UnExecute()
    {
        //basically undo the command 
        //if pX is zero, un merge the row
        if(pX == 0){
          
            for(int i = 0; i < removed_chars.size(); i++)
            {
                int j = old_len+1;
                model.RemoveCharAt(j, pY-1, bound);
                j++;
            }
            
            model.AddRowAt(pY);
            for(int i = 0; i < removed_chars.size(); i++)
            {
                model.AddCharAt(removed_chars[i], i , pY, bound);
            }  
            removed_chars.clear();
        }
        else {
        char c  = removed_chars.back();
        //was pX-1
        model.AddCharAt(c, pX-1, pY, bound);
        removed_chars.pop_back();
        }
    }

private:
    Model &model;
    int pX;
    int pY;
    int bound;
    //keep a list of the chars removed from the model
    std::vector<char> removed_chars;

    //keep track of the size of the old row
    //so when we unexecute, we know how much to erase
    int old_len;
};


//concrete command object for enter
class CMD_Enter: public Command
{

public: 
    CMD_Enter(Model &m, int posX, int posY, int b) : model(m), x(posX), y(posY), bound(b) {}
    ~CMD_Enter(){}

    void Execute()
    {
        //if x is in between a row, add a new row and add split text to new row
        //push other rows back
        if(x <= model.GetCurrRowSize(y)-1 || x == 0 || x >= model.GetCurrRowSize(y))
        {
            //get the the text to be removed
            removed_text = model.GetRow(y).substr(x, model.GetRow(y).size() - x);
            //remove the text from the current row
            std::vector<std::string> temp = model.GetText();
            temp[y].erase(temp[y].begin() + x, temp[y].end());
            model.SetText(temp);

            model.AddRowAt(y+1);
            model.AddTextAt(removed_text, y+1);
        }
        //case where if y is in range of the text size and is not zero
        //we just push the rows past y
    }

    void UnExecute()
    {
        //add the removed text to the current row
        
        model.RemoveRow(y+1);
        std::vector<std::string> temp = model.GetText();
        temp[y] += removed_text;
        model.SetText(temp);
    }

private:
    Model &model;
    int x;
    int y;
    int bound;
    //keep track of the text that was removed from a row
    std::string removed_text;
};
//************************************************************************************

//creating the controller class


class Controller
{

//call the commands to do the our bidding
public:
    Controller(Model &m, ECTextViewImp &obj): model(m), view(obj)
    { 
        mode = 0; 
        file_name = model.GetFileName(); 
        curr_session = -1;
        index_undo = 0;
        index_redo = 0;

        x = view.GetCursorX();
        y = view.GetCursorY();
     
    }
    ~Controller()
    {
        //free the created command objects
        for(auto it = list_CMD.begin(); it != list_CMD.end(); it++)
        {
            for(auto it2 = it->begin(); it2 != it->end(); it2++)
            {
                delete (*it2);
            }
        }

        for(auto it = list_undone.begin(); it != list_undone.end(); it++)
        {
            for(auto it2 = it->begin(); it2 != it->end(); it2++)
            {
                delete (*it2);
            }
        }

    }
 //************************** METHODS THAT MODIFY THE MODEL  ***************************************************************************************************************************************************************************
    //create and call each of the concrete commands
    //add char by char
    void AddChar(char c)
    {
        if(mode)
        //call the concrete command class to do this
        {
        
            AddTextAt * add = new AddTextAt(model, c, x, y, col_lim);
            add->Execute();
            //list_CMD.push_back(add);
            
            if(list_CMD.size() == curr_session)
            {
                list_CMD.emplace_back();
                list_CMD.back().emplace_back(add);
            }
            else list_CMD[curr_session].emplace_back(add);
            
     
            MoveCurRight();
            AddView();
            WriteFile();
            view.Refresh();

        }
    }


    void DeleteChar()
    {
        //only delete a char at if the mode is 1

        if(mode)
        {
            //in the case that x is 0
            //keep track o
            std::vector<std::string> v1 = model.GetText();
            int newXPos = v1[y-1].size();


            int size_before = model.GetTextSize();
            RemoveTextAt * rem = new RemoveTextAt(model, x, y, col_lim);
            rem->Execute();
            //list_CMD.push_back(rem);
            
            if(list_CMD.size() == curr_session)
            {
                list_CMD.emplace_back();
                list_CMD.back().emplace_back(rem);
            }
            else list_CMD[curr_session].emplace_back(rem);

        //get the new text from the model
        //add to view

            std::vector<std::string> temp = model.GetText();
        //view.Refresh();
            //view.InitRows();
            
        
            int size = temp.size();
        //view.SetCursorX(temp[size].size());
            //was y < 63 before
            if(y > temp.size()-1 && y < view.GetRowNumInView())
            {
                MoveCurUp();

                for(int i = 0; i < newXPos; i++) MoveCurRight();

            }
            else MoveCurLeft();
            AddView();
            WriteFile();
            view.Refresh();

        }
    }
    
    void Enter()
    {
        //case where we press enter in the middle of a row that has text
        if(mode)
        {
            CMD_Enter * enter = new CMD_Enter(model, x, y, col_lim);
            enter->Execute();
            //list_CMD.push_back(enter);
            if(list_CMD.size()  == curr_session)
            {
                list_CMD.emplace_back();
                list_CMD.back().emplace_back(enter);
            }
            else list_CMD[curr_session].emplace_back(enter);

            //move down if 
            MoveCurDown();
            view.SetCursorX(0);
            AddView();
            WriteFile();
            view.Refresh();
        }
    }
//************************************************************************************************************************************************************************************************************************************************
//****************************** METHODS THAT MODIFY CURSOR POSTION  *****************************************************************************************************************************************************************************************************

    void MoveCurRight()
    {
        std::vector<std::string> temp = model.GetText();

        //handle case for text wrapping
        if(temp[y].size() > col_lim)
        {
            //if the cursor is at the end of the column limit, move the cursor down and set the cursor at the beginning
            if(view.GetCursorX() == col_lim)
            {
                view.SetCursorY(view.GetCursorY()+1);
                view.SetCursorX(0);
                x+=1;
            }
            else if(x < temp[y].size())
            {
                x+=1;
                view.SetCursorX(view.GetCursorX()+1);

                //view.AddRow(std::to_string(view.GetCursorX()));
            }
        }
        //no text wrapping, just move to the right
        else
        {
            if(x < temp[y].size())
            {
                x +=1;
                view.SetCursorX(view.GetCursorX()+1);
            }
        }
        UpdateStatusBar();
    }

    void MoveCurLeft()
    {
        std::vector<std::string> temp = model.GetText();
        //handle case for text wrapping
        if(temp[y].size() > col_lim)
        {
            //if currently in a wrapped row, we must move the cursor up to the last position
            if(view.GetCursorX() == 0 && x!= 0)
            {
                view.SetCursorY(view.GetCursorY()-1);
                view.SetCursorX(col_lim-1);
                x-=1;
            }
            else
            {
                view.SetCursorX(view.GetCursorX()-1);
                x-=1;
            }
        }
        //no text wrapping
        else
        {
            if(x > 0)
            {
                x-=1;
                view.SetCursorX(view.GetCursorX()-1);
            }
        }
        UpdateStatusBar();
    }

    void MoveCurDown()
    {
        //just move down a row
        std::vector<std::string> temp = model.GetText();
        if(view.GetCursorY() < row_lim && y < temp.size()-1)
        {
            //case of wrapped text
            if(temp[y].size() > col_lim)
            {
                int n = temp[y].size()/col_lim;
                view.SetCursorY(view.GetCursorY() + (n+1));
                view.SetCursorX(0);

                //adjust the x and y positions
                y+=1;
                x = 0;
            }
            else
            {
                //if the current x pos is invalid for the next row, adjust set the x position to the end of the next row
                if(x > temp[y+1].size())
                {
                    x = temp[y+1].size();
                    view.SetCursorX(x);
                }
                view.SetCursorY(view.GetCursorY() + 1);
                y+=1; 
            }
        }
        UpdateStatusBar();
    }

    void MoveCurUp()
    {   
        std::vector<std::string> temp = model.GetText();
        if(y>0)
        {
            //in case of text wrapping
            if(temp[y-1].size() > col_lim)
            {
                int n = temp[y-1].size()/col_lim;
                view.SetCursorY(view.GetCursorY() - (n+1));
                view.SetCursorX(0);
                y -= 1;
                x = 0;
            }
            else
            {
                if(x > temp[y-1].size())
                {
                    x = temp[y-1].size();
                    view.SetCursorX(x);
                }
                y-=1;
                view.SetCursorY(view.GetCursorY()-1);
            }
        }
        UpdateStatusBar();
    }

    //correct the cursor when doing 
    void Correct_Cursor()
    {
        int num_rows = model.GetText().size();
        int cur_x = view.GetCursorX();
        int cur_y = view.GetCursorY();

        if(cur_y >= num_rows)
        {
            cur_y = num_rows-1;
        }
        int len = model.GetText()[cur_y].length();

        if(cur_x > len) cur_x = len;

        view.SetCursorX(cur_x);
        view.SetCursorY(cur_y);

        x = cur_x;
        y = cur_y;
    }

    std::vector<std::string> Text_RV()
    {
        return model.GetText();
    }
    //change the mode
    void ChangeMode(bool var) { mode = var; }

    bool GetMode() const { return mode; }

    //set the file of the output file



//**************************   UNDO/REDO   ***************************************************************************************************************************************************************************************************************
//for both the undo/redo, we need to unexecute/execute the commands that are in the back of the vector of pointers to the command objects

    void Undo()
    {
        if(!mode)
        {
            index_undo = list_undone.size();
            
            if(list_CMD.empty()) { return;}
            //undo commands from the last session
            //go to the most recent session
            auto i = list_CMD.rbegin();
            //loop through the session and unexecute the most recent command           
            for(auto j = i->rbegin(); j != i->rend(); j++)
            {
                (*j)->UnExecute();
                //correct the cursor postion after each undo
                Correct_Cursor();
                if(list_undone.size() == index_undo)
                {
                    list_undone.emplace_back();
                    list_undone.back().emplace_back(*j);
                }
                else list_undone[index_undo].push_back(*j);
            }
           
            index_undo++;
            curr_session--;
    
            //remove the last session from the list
            list_CMD.pop_back();
            AddView();
            //Correct_Cursor();
            WriteFile();
        }
    }

    void Redo()
    {
        if(!mode)
        {
            //same logic as undo except we execute
            index_redo = list_CMD.size();
            if(list_undone.empty()) {return;}
            
            auto i = list_undone.rbegin();
            for(auto j = i->rbegin(); j != i->rend(); j++)
            {
                (*j)->Execute();
                Correct_Cursor();
                if(list_CMD.size() == index_redo)
                {
                    list_CMD.emplace_back();
                    list_CMD.back().emplace_back(*j);
                }
                else list_CMD[index_redo].push_back(*j);
            }
            index_redo++;
            curr_session--;
        

            list_undone.pop_back();            
            AddView();
            //Correct_Cursor();
            WriteFile();
        }

    }

//************************ METHODS THAT DIRECTLY MODIFY VIEW ****************************************************************************************************************************************************************************************************

    //method to wrap the text

    void WrapText(std::string &text)
    {
        std::string str = text;
        while(!str.empty())
        {
            int str_len = std::min(view.GetColNumInView()-1, static_cast<int>(str.length()));
            view.AddRow(str.substr(0, str_len));
            str = str.substr(str_len);
        }
            
    }



    //text highlighting
    void Highlight()
    {   //get the text
        std::vector<std::string> temp = model.GetText();
        int ybound = view.GetRowNumInView();
        //creat a vector of strings that contains the words to be highlighted
       
        std::vector<std::string> vec;
        std::ifstream key("keywords.txt");

        if (key.is_open())
        {
            //read the keywords and push back to vec
            std::string str;
            //for some reason, getline was buggy when reading from keywords.txt,
            //instead of reading line and pushing into vector, add each element of the string into
            //another string, once done push that string into the vector
            while (getline(key, str))
            {
                std::string n = "";
                for(int i = 0; i < (int)str.size()-1; i++)
                {
                    n += str[i];
                }
                vec.push_back(n);
            }
        
            key.close();
            //view.AddRow(vec[0]);
        
        }
        if(!vec.empty())
        {
        
            int max_len = std::min(ybound, static_cast<int>(temp.size()));

            for(int row_num = 0; row_num < max_len; row_num++)
            {
                
                std::string str = temp[row_num];

                std::string::size_type index = 0;

                while(index < str.size())
                {
                    while(index < str.size() && str[index] == ' ')
                    {
                        index++;
                    }

                    std::string::size_type end = str.find(' ', index);

                    if(end == std::string::npos) end = (int)str.size();

                    std::string word = str.substr(index, end - index);
                    if(std::find(vec.begin(), vec.end(), word) != vec.end())
                    {
                        //view.AddRow("Yes");
                        view.SetColor(row_num, index, end-1, TEXT_COLOR_BLUE);
                    }
                    index = end+1;
                }

            }
        }
        //simply return if there are no keywords in the dic
        else return;    
    }


    //method to update the status bar to display the x and y coordinates

    void UpdateStatusBar()
    {
        //update the status bar with updated coordinates with the proper mode message
        view.ClearStatusRows();
        if(mode)
        {
            std::string msg = "Current Mode: Edit    ";
            msg += "Column = ";
            msg+= std::to_string(x);
            msg+= ", Row = ";
            msg+= std::to_string(y);
            view.AddStatusRow(msg, "Press ESC for Command Mode", true);
            view.Refresh();
        }
        else
        {
            std::string msg = "Current Mode: Command    ";
            msg += "Column = ";
            msg+= std::to_string(x);
            msg+= ", Row = ";
            msg+= std::to_string(y);
            view.AddStatusRow(msg, "Press i for Edit Mode", true);
            view.Refresh();

        }
    }

    //method to update the view
    void AddView()
    {
        view.InitRows();
        view.ClearColor();
        std::vector<std::string> temp = model.GetText();

        int num_rows = std::min(row_lim, static_cast<int>(temp.size()));
    
        for(int i = 0; i < num_rows; i++)
        {
            if(temp[i].size() > col_lim)
            {
                WrapText(temp[i]);
            }
            else view.AddRow(temp[i]);
        }
        //highlight the key words
        Highlight();
        
        view.Refresh();

    }

    //function to add to the file after modifying the model and view
    void WriteFile()
    {
        //get the text from the view and add to the file
        std::vector<std::string> temp = model.GetText();
        std::ofstream outfile;
        outfile.open(file_name);
        for(auto it: temp) outfile << it << std::endl;
        outfile.close();
        
    }


    //increase the session number
    void IncSession() { curr_session += 1; }


//keep an instance of the model
private:
    Model &model;
    ECTextViewImp &view;

    //store the name of the output file to then open to write to
    std::string file_name;

    //var that keeps track of current mode
    //0 for command and 1 for edit
    //by default its 0
    bool mode;

    //
    int index_undo;
    int index_redo;

    //maintain a list of the commands to then delete them
    std::vector<std::vector<Command *>> list_CMD;
    std::vector<std::vector<Command *>> list_undone;

    //keep track of the coordinates in relation to the model
    int x,y;

    //keep track of the number of sessions
    int curr_session;

    //row and column limit of the window
    const int row_lim = view.GetRowNumInView();
    const int col_lim = view.GetColNumInView();
    

    
};
//*********************************************************************************************

#endif
