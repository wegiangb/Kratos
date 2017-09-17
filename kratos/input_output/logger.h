//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics 
//
//  License:         BSD License 
//                     Kratos default license: kratos/license.txt
//
//  Main authors:    Pooyan Dadvand
//                   Carlos Roig 
//                   Vicente Mataix Ferrandiz
//
               

#if !defined(KRATOS_LOGGER_H_INCLUDED )
#define  KRATOS_LOGGER_H_INCLUDED

// System includes
#include <string>
#include <iostream> 

// Project includes
#include "input_output/logger_table.h"
#include "input_output/logger_message.h"
#include "input_output/logger_output.h"
#include "includes/exception.h"

namespace Kratos
{
  ///@addtogroup Kratos
  ///@{

  ///@name Kratos Macros
  ///@{ 

  ///@} 
  ///@name Type Definitions
  ///@{ 
  
  ///@} 
  ///@name  Enum's
  ///@{
      
  ///@}
  ///@name  Functions 
  ///@{
      
  ///@}
  ///@name Kratos Classes
  ///@{
  
  /// Logger is in charge of writing the messages to output streams.
  /** Logger is the main class in message writing pipeline which holds an 
      array of logger outputs and dispach the arriving logger messages 
      to them. Implements a singletone for the list of the outputs and
      also has public constructors and destructors to perform the 
      streaming.
  */
  class KRATOS_API(KRATOS_CORE) Logger
  {
  public:
      ///@name Type Definitions
      ///@{
      
      typedef TableStream TableStreamType;
      
      using LoggerOutputContainerType = std::vector<LoggerOutput>;
      
      ///@}
      ///@name Enums
      ///@{

      using Severity = LoggerMessage::Severity;

      using Category = LoggerMessage::Category;

      ///@}
      ///@name Life Cycle 
      ///@{ 
      
      /// Default constructor.
      Logger(const bool UseTable = false);


      /// Avoiding Logger to be copied
      Logger(Logger const& rOther) = delete;


      /// Destructor is in charge of passing the message into outputs
      virtual ~Logger();

      ///@}
      ///@name Operators 
      ///@{
      
      /// Loggers can not be assigned.
      Logger& operator=(Logger const& rOther) = delete;

      ///@}
      ///@name Operations
      ///@{
      
      
      ///@}
      ///@name Static Methods
      ///@{
      
      static LoggerOutputContainerType& GetOutputsInstance()
      {
          static LoggerOutputContainerType instance;
          return instance;
      }

      static void AddOutput(LoggerOutput const& TheOutput);

      ///@}
      ///@name Access
      ///@{ 

      void SetUseTable(const bool UseTable)
      {
          mUseTable = UseTable;
      }
      
      std::string const& GetCurrentMessage() 
      {
          return mCurrentMessage.GetMessage();
      }
      
      TableStream const& GetCurrentTable() 
      {
          return mCurrentTable.GetTable();
      }
      
      /**
       * This function prints the header of the table
       */
       void StartTable()
       {
           if (mUseTable == true) mCurrentTable.PrintTableHeader();
       }
        
      /**
       * This function prints the footer of the table
       */
       void EndTable()
       {
           if (mUseTable == true) mCurrentTable.PrintTableFooter();
       }
        
      /**
       * It adds a column to the table
       * @param ThisName: The name of the variable
       * @param ThisSpaces: The number of spaces to consider
       */
      void AddColumnToTable(        
        std::string ThisName, 
        const unsigned int ThisSpaces = 10
        )
      {
          if (mUseTable == true) mCurrentTable.AddColumnToTable(ThisName, ThisSpaces);
      }
        
      /**
       * This function sets if the table uses the bold UseBoldFont
       * @param UseBoldFont: If the bold font is used
       */
      void SetBoldTable(const bool UseBoldFont) 
      {
          if (mUseTable == true) mCurrentTable.SetBoldTable(UseBoldFont);
      }
      
      ///@}
      ///@name Inquiry
      ///@{
      
      
      ///@}      
      ///@name Input and output
      ///@{

      /// Turn back information as a string.
      virtual std::string Info() const;
      
      /// Print information about this object.
      virtual void PrintInfo(std::ostream& rOStream) const;

      /// Print object's data.
      virtual void PrintData(std::ostream& rOStream) const;
      
      /// string stream function
      template<class StreamValueType>
      Logger& operator << (StreamValueType const& rValue)
      {
          if (mUseTable == false)
          {
              mCurrentMessage << rValue;
          }
          else
          {
              mCurrentTable << rValue;
          }

          return *this;
      }

      /// Manipulator stream function
      Logger& operator << (std::ostream& (*pf)(std::ostream&));

      /// char stream function
      Logger& operator << (const char * rString);

      /// Severity stream function
      Logger& operator << (Severity const& TheSeverity);

      /// Category stream function
      Logger& operator << (Category const& TheCategory);

      ///@}      
  private:
      ///@name Static Member Variables 
      ///@{ 
        
      ///@} 
      ///@name Member Variables 
      ///@{ 
        
      LoggerMessage mCurrentMessage;
      
      LoggerTable mCurrentTable;
      
      bool mUseTable;
        
      ///@} 
      ///@name Private Operators
      ///@{ 
        
        
      ///@} 
      ///@name Private Operations
      ///@{ 
        
        
      ///@} 
      ///@name Private  Access 
      ///@{ 
        
        
      ///@}    
      ///@name Private Inquiry 
      ///@{ 
        
        
      ///@}    
      ///@name Un accessible methods 
      ///@{ 
      
       
      ///@}    
        
    }; // Class Logger 

  ///@} 
  
  ///@name Type Definitions       
  ///@{ 
  
  
  ///@} 
  ///@name Input and output 
  ///@{ 
        
 
  /// input stream function
  inline std::istream& operator >> (std::istream& rIStream, 
                    Logger& rThis);

  /// output stream function
  inline std::ostream& operator << (std::ostream& rOStream, 
                    const Logger& rThis)
    {
      rThis.PrintInfo(rOStream);
      rOStream << std::endl;
      rThis.PrintData(rOStream);

      return rOStream;
    }
  ///@}

  ///@} addtogroup block

}  // namespace Kratos.

#endif // KRATOS_LOGGER_H_INCLUDED  defined 


