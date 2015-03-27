/**********************************************************************
 * Assignment:
 * Sanitization Program
 * Author:
 * Schuyler Summers
 * Summary:
 * Takes HTML input and removes tags that are not on the white list
**********************************************************************/
#include <iostream>
#include <set>
#include <string>
#include <stack>
using namespace std;

const string VALID [] = {"a", "abbr", "acronym", "b", "blockquote", "cite",
                         "code", "del", "em", "i", "q", "strike", "strong"};
const set<string> VALID_TAGS(VALID, VALID+13);

string getTagType(string);
string scrubTag(string);

/**********************************************************************
 * Main asks for input, sanatizes the input, then prompts for input
 * until that input is "quit"
 **********************************************************************/
int main(int argc, char** argv)
{
   stack<string> openTags;
   string input = "";
   string output = "";
   string tag = "";
   string tagType;
   string openedTag;
   
   // Input loop
   while (input != "quit")
   {
      cout << "> ";
      getline(cin, input);
      cout << "\t";

      // Iterate through each letter of the input
      for (string::iterator it = input.begin(); it != input.end(); ++it)
      {
         // If it finds the start of the tag
         if(*it == '<')
         {
            tag = *it;
            // Read in until the end of the tag
            do
            {
               it++;
               tag += *it;
            } while(*it != '>');
                        
            tagType = getTagType(tag);
                        
            // If it is a valid tag
            if (VALID_TAGS.find(tagType) != VALID_TAGS.end())
            {
               // If it is a close tag
               if (tag.find("/") == 1)
               {
                  // Make sure people don't close tags they didn't open
                  if (!openTags.empty())
                     openedTag = openTags.top();
                  else
                     openedTag = "";

                  // If the tag doesn't close the right tag then scrub it
                  // otherwise consider the tag closed
                  if (openedTag != tagType)
                     tag = scrubTag(tag);
                  else
                     openTags.pop();
               }
               else
               {
                  // If a tag is opened put it on the stack of opened tags
                  openTags.push(tagType);
               }
            }
            else
            {
               // If it's not valid remove '<' and '>'
               tag = scrubTag(tag);
            }
            
            output = tag;
         }
         else
         {
            // If it's not a tag just make the letter the output
            output = *it;
         }
         
         cout << output;
      }

      // Close up all the tags left open in order
      while (!openTags.empty())
      {
         cout << "</" << openTags.top() << ">";
         openTags.pop();
      }
      
      cout << endl;
   }
}

/**********************************************************************
 * getTagType gets the tag type out of a tag and returns it as a string
 **********************************************************************/
string getTagType(string tag)
{
   // If the tag is closing start after the "/"
   int startPos = tag.find("/");
   if (startPos == -1)
   {
      startPos = tag.find("<");
   }

   // Some tags have spaces after the tag type
   int endPos = tag.find(" ");
   if (endPos == -1)
   {
      endPos = tag.find(">");
   }

   return tag.substr(startPos + 1, endPos - startPos - 1);
}

/**********************************************************************
 * scrubTag takes a string and replaces the "<" and ">" sybmols with
 * "&lt;" and "&gt;"
 **********************************************************************/
string scrubTag(string tag)
{
   string result = "";
   for (string::iterator it = tag.begin(); it != tag.end(); ++it)
   {
      switch(*it)
      {
         case '<':
            result += "&lt;";
            break;
         case '>':
            result += "&gt;";
            break;
         default:
            result += *it;
      }
   }

   return result;
}
