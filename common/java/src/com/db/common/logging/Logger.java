/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.logging;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.io.StringWriter;

import java.text.DateFormat;
import java.util.Date;
import java.util.Hashtable;
import java.util.Iterator;

/**
 * A class that handles logging to a file.
 *  
 * @author Dave Longley
 */
public class Logger
{
   /**
    * The name of this logger.
    */
   protected String mName;
   
   /**
    * The current verbosity setting for the log file.
    */
   protected int mFileVerbosity;
   
   /**
    * The current verbosity setting for the console.
    */
   protected int mConsoleVerbosity;

   /**
    * The date format.
    */
   protected String mDateFormat;
   
   /**
    * The filename for the log file.
    */
   protected String mFilename;

   /**
    * The file output stream to write logging information to.
    */
   protected PrintStream mStream;
   
   /**
    * The maximum file size for the log file.
    */
   protected long mMaxFileSize;
   
   /**
    * The id of the next log file to rotate out.
    */
   protected long mRotateId;
   
   /**
    * The number of rotating files. This is the number of files, other
    * than the main log file that can be rotated in/out in case a write
    * to a log file would exceed the maximum log size.
    */
   protected long mNumRotatingFiles;
   
   /**
    * A map of all of the logging print streams to their verbosities. 
    */
   protected Hashtable mStreamToVerbosity;
   
   /**
    * The default number of log files to rotate.
    */
   public static final long DEFAULT_NUM_ROTATING_FILES = 3;
   
   public static final int NO_VERBOSITY = 0;
   public static final int ERROR_VERBOSITY = 1;
   public static final int WARNING_VERBOSITY = 2;
   public static final int MSG_VERBOSITY = 3;
   public static final int DEBUG_VERBOSITY = 4;
   public static final int MAX_VERBOSITY = DEBUG_VERBOSITY;

   /**
    * Creates a new logger with default verbosity.
    * 
    * @param name the name of the logger.
    */
   public Logger(String name)
   {
      this(name, Logger.DEBUG_VERBOSITY);
   }

   /**
    * Creates a new logger with specified verbosity.
    *
    * @param name the name of the logger.
    * @param fileVerbosity the max verbosity to display in the log file.
    */
   public Logger(String name, int fileVerbosity)
   {
      this(name, fileVerbosity, Logger.NO_VERBOSITY);
   }
   
   /**
    * Creates a new logger with specified verbosity.
    *
    * @param name the name of the logger.
    * @param fileVerbosity the max verbosity to display in the log file.
    * @param consoleVerbosity the max verbosity to display in the console.
    */
   public Logger(String name, int fileVerbosity, int consoleVerbosity)
   {
      mName = name;
      mFileVerbosity = fileVerbosity;
      mConsoleVerbosity = consoleVerbosity;
      mStream = null;
      mDateFormat = "MM/dd/yy H:mm:ss";
      mMaxFileSize = -1;
      mRotateId = -1;
      mNumRotatingFiles = DEFAULT_NUM_ROTATING_FILES;
      mStreamToVerbosity = new Hashtable();
   }
   
   /**
    * Overloaded to ensure that the stream gets closed when garbage
    * collected.
    */
   protected void finalize()
   {
      closeStream();
   }

   /**
    * Closes the output stream if it is open.
    */
   protected synchronized void closeStream()
   {
      if(mStream != null)
      {
         // remove stream from stream verbosity map
         mStreamToVerbosity.remove(mStream);
         
         try
         {
            mStream.close();
         }
         catch(Exception e)
         {
         }
         
         mStream = null;
      }
   }

   /**
    * Gets the current date in the appropriate format.
    * 
    * @return the current date in the appropriate format.
    */
   protected String getDate()
   {
      String date = "";

      if(mDateFormat.equals(""))
      {
         // do nothing
      }
      else
      {
         // handle other date formats here
         Date now = new Date();
         
         if(mDateFormat.equals("day mon yy hh:mm:ss yyyy"))
         {
            date = now.toString();
         }
         else if(mDateFormat.equals("MM/dd/yy H:mm:ss"))
         {
            date = DateFormat.getDateTimeInstance(
                   DateFormat.SHORT, DateFormat.LONG).format(now); 
         }
         else
         {
            date = now.toString();
         }
      }

      return date;
   }
   
   /**
    * Gets the id of the next log file to rotate out. Auto-increments for
    * the next call.
    * 
    * @return the id of the next log file to rotate out. 
    */
   protected long getRotateId()
   {
      mRotateId++;
      
      if(getNumRotatingFiles() != -1 && mRotateId >= getNumRotatingFiles())
      {
         mRotateId = 0;
      }
      
      return mRotateId;
   }
   
   /**
    * Rotates the log file as necessary. If the next append to a log file
    * would exceed its maximum size, then the log file is rotated out.
    * This method will only make changes to the log file if there has been
    * a maximum log file size set.
    * 
    * @param logText the log text to be appended to the log file.
    * @return the adjusted log text, if it cannot fit into a single
    *         log file.
    */
   protected synchronized void rotateLogFile(String logText)
   {
      if(getMaxFileSize() > 0)
      {
         // includes end line character
         int logTextLength = logText.length() + 1;
         
         File file = new File(mFilename);
         long newLength = file.length() + logTextLength;
         long overflow = newLength - getMaxFileSize();
         if(overflow > 0)
         {
            // there is overflow, so rotate the files
            String rotateLog = file.getAbsolutePath() + "." + getRotateId();
            File newFile = new File(rotateLog);
            
            // if there is no limit on the number of rotating files,
            // then do not overwrite existing rotated logs
            if(getNumRotatingFiles() == -1)
            {
               // keep going until an unused file is found
               while(newFile.exists())
               {
                  rotateLog = file.getAbsolutePath() + "." + getRotateId();
                  newFile = new File(rotateLog);
               }
            }

            // ensure the new file does not exist
            newFile.delete();

            // rename current log file to new file
            file.renameTo(newFile);
            
            // reset the log files
            LoggerManager.resetLoggerFiles(mFilename);
         }
      }
   }
   
   /**
    * Gets the name of this logger.
    * 
    * @return the name of this logger.
    */
   public String getName()
   {
      return mName;
   }

   /**
    * Sets the file verbosity for this logger. If the verbosity
    * passed is not in the accepted range, this method will
    * fail to set the verbosity and return false. Otherwise
    * it will set the verbosity and return true.
    *
    * @param fileVerbosity the verbosity to set.
    * @return true if verbosity valid and set, false if not.
    */
   public synchronized  boolean setFileVerbosity(int fileVerbosity)
   {
      boolean rval = false;

      if(fileVerbosity > NO_VERBOSITY && fileVerbosity <= MAX_VERBOSITY)
      {
         mFileVerbosity = fileVerbosity;
         
         if(getPrintStream() != null)
         {
            if(mFileVerbosity == NO_VERBOSITY)
            {
               mStreamToVerbosity.remove(getPrintStream());
            }
            else
            {
               mStreamToVerbosity.put(getPrintStream(),
                                      new Integer(mFileVerbosity));
            }
         }
         
         rval = true;
      }

      return rval;
   }

   /**
    * Gets the file verbosity set for this logger.
    *
    * @return the file verbosity set for this logger.
    */
   public int getFileVerbosity()
   {
      return mFileVerbosity;
   }
   
   /**
    * Sets the console verbosity for this logger. If the verbosity
    * passed is not in the accepted range, this method will
    * fail to set the verbosity and return false. Otherwise
    * it will set the verbosity and return true.
    *
    * @param consoleVerbosity the verbosity to set.
    * @return true if verbosity valid and set, false if not.
    */
   public synchronized boolean setConsoleVerbosity(int consoleVerbosity)
   {
      boolean rval = false;

      if(consoleVerbosity >= NO_VERBOSITY && consoleVerbosity <= MAX_VERBOSITY)
      {
         mConsoleVerbosity = consoleVerbosity;
         
         if(System.out != null)
         {
            if(mConsoleVerbosity == NO_VERBOSITY)
            {
               mStreamToVerbosity.remove(System.out);
            }
            else
            {
               mStreamToVerbosity.put(System.out,
                                      new Integer(mConsoleVerbosity));
            }
         }
         
         rval = true;
      }

      return rval;
   }

   /**
    * Gets the console verbosity set for this logger.
    *
    * @return the console verbosity set for this logger.
    */
   public int getConsoleVerbosity()
   {
      return mConsoleVerbosity;
   }

   /**
    * Sets the date format. If the date format given is not
    * a valid format, the method does nothing but return false.
    *
    * @param dateFormat the new date format.
    * @return true if the date format is set, false if not.
    */
   public synchronized boolean setDateFormat(String dateFormat)
   {
      boolean rval = false;

      if(dateFormat == "")
      {
         rval = true;
      }
      else
      {
         // other formats here
         mDateFormat = dateFormat;
      }

      return rval;
   }
   
   /**
    * Opens a new log file with the specified file name. Will append to
    * the file if it already exists.
    *
    * @param filename the name of the file to log to.
    * @return true if succesfully opened the file for writing, false if not.
    */
   public boolean setFile(String filename)
   {
      return setFile(filename, true);
   }   

   /**
    * Opens a new log file with the specified file name. Setting append to
    * true will append the file if it exists. Setting it to false will
    * overwrite it.
    *
    * @param filename the name of the file to log to.
    * @param append specifies whether or not to append to an existing
    *             file or to overwrite.
    * @return true if succesfully opened the file for writing, false if not.
    */
   public synchronized boolean setFile(String filename, boolean append)
   {
      boolean rval = false;
      
      closeStream();
      
      if(!append)
      {
         File file = new File(filename);
         file.delete();
      }
      
      mFilename = filename;
      
      try
      {
         FileOutputStream fos = new FileOutputStream(filename, true);
         mStream = new PrintStream(fos);
         
         // add stream to stream verbosity map
         mStreamToVerbosity.put(mStream, new Integer(this.getFileVerbosity()));
         
         rval = true;
      }
      catch(Exception e)
      {
      }

      return rval;
   }
   
   /**
    * Sets the maximum log file size. Setting the maximum log file
    * size to -1 means that there is no maximum.
    * 
    * @param fileSize the maximum log file size. -1 for no maximum.
    */
   public synchronized void setMaxFileSize(long fileSize)
   {
      // -1 is means no maximum log file size
      mMaxFileSize = Math.max(-1, fileSize);
   }
   
   /**
    * Gets the maximum log file size.
    * 
    * @return the max log file size.
    */
   public long getMaxFileSize()
   {
      return mMaxFileSize;
   }
   
   /**
    * Sets the number of rotating log files. This is the number of files
    * other than the main log file that may be rotated in when the
    * maximum log file size would otherwise be exceeded. No fewer than
    * 1 file may be set. If a value of less than zero is passed, then
    * there will be no limit on the number of rotating files.
    *
    * @param numRotatingFiles the number of rotating log files.
    * @return true if successfully set, false if not.
    */
   public synchronized boolean setNumRotatingFiles(long numRotatingFiles)
   {
      boolean rval = false;
      
      if(numRotatingFiles != 0)
      {
         mNumRotatingFiles = Math.max(-1, numRotatingFiles);
         rval = true;
      }
      
      return rval;
   }

   /**
    * Gets the number of rotating log files. This is the number of files
    * other than the main log file that may be rotated in when the
    * maximum log file size would otherwise be exceeded.
    *
    * @return the number of rotating log files.
    */
   public long getNumRotatingFiles()
   {
      return mNumRotatingFiles;
   }
   
   /**
    * Gets the filename set for this logger.
    * 
    * @return the filename set for this logger.
    */
   public String getFilename()
   {
      return mFilename;
   }
   
   /**
    * Gets the print stream for this logger.
    * 
    * @return the print stream for this logger.
    */
   public PrintStream getPrintStream()
   {
      return mStream;
   }
   
   /**
    * Gets a print writer for this logger.
    * 
    * @return a print writer for this logger.
    */
   public PrintWriter getPrintWriter()
   {
      PrintWriter pw = null;
      
      try
      {
         pw = new PrintWriter(getPrintStream());
      }
      catch(Throwable t)
      {
      }
      
      return pw;
   }   
   
   /**
    * Adds a print stream to the logger.
    * 
    * @param ps the print stream to add.
    * @param verbosity the verbosity for the print stream.
    */
   public synchronized void addPrintStream(PrintStream ps, int verbosity)
   {
      setPrintStreamVerbosity(ps, verbosity);
   }
   
   /**
    * Removes a print stream from the logger.
    * 
    * @param ps the print stream to remove.
    */
   public synchronized void removePrintStream(PrintStream ps)
   {
      if(ps != null)
      {
         mStreamToVerbosity.remove(ps);
      }
   }
   
   /**
    * Sets a print stream's verbosity.
    * 
    * @param ps the print stream to modify.
    * @param verbosity the verbosity for the print stream.
    */
   public synchronized void setPrintStreamVerbosity(PrintStream ps,
                                                    int verbosity)
   {
      if(ps != null)
      {
         if(ps == getPrintStream())
         {
            setFileVerbosity(verbosity);
         }
         else if(ps == System.out)
         {
            setConsoleVerbosity(verbosity);
         }
         else
         {
            mStreamToVerbosity.put(ps, new Integer(verbosity));
         }
      }
   }
   
   /**
    * Writes the passed string to the log file, if it is open.
    *
    * @param text the text to write to the log file.
    * @return true if the text was written, false if not.
    */
   public boolean log(String text)
   {
      return log(text, Logger.ERROR_VERBOSITY);
   }
   
   /**
    * Writes the passed string to the console/log file, if it is open.
    *
    * @param text the text to write to the log file.
    * @param verbosity the verbosity level that must be reached in
    *                  order for the text to be written to the log.
    * @return true if the text was written, false if not.
    */
   public synchronized boolean log(String text, int verbosity)
   {
      return log(text, verbosity, true, true);
   }
   
   /**
    * Writes the passed string to the console/log file, if it is open.
    *
    * @param text the text to write to the log file.
    * @param verbosity the verbosity level that must be reached in
    *                  order for the text to be written to the log.
    * @param header true to use the logger's header, false not to.
    * @param useCustomStreams true to print to custom streams, false to only
    *                         print to console/log file.
    * @return true if the text was written, false if not.
    */
   public synchronized boolean log(String text, int verbosity, boolean header,
                                   boolean useCustomStreams)
   {
      boolean rval = false;
      
      if(mStreamToVerbosity.size() != 0)
      {
         String logText = ""; 
         
         if(header)
         {
            String date = getDate();
            if(!date.equals(""))
            {
               logText = getDate() + " " + getName() + " - " + text;
            }
            else
            {
               logText = getName() + " - " + text;
            }
         }
         else
         {
            logText = text;
         }
         
         String logFileText = logText;
         
         // if entire log text cannot be entered, break it up
         String remainder = "";
         if(getMaxFileSize() > 0 &&
            (logText.length() + 1) > getMaxFileSize())
         {
            remainder = logText.substring((int)getMaxFileSize());
            logFileText = logText.substring(0, ((int)getMaxFileSize()) - 1);
         }
         
         // lock on the loggermanager
         synchronized(LoggerManager.getInstance())
         {
            // ensure a file is set if appropriate
            if(getMaxFileSize() != 0 &&
               mFilename != null && !mFilename.equals(""))
            {
               File file = new File(mFilename);
               
               // if the file no longer exists, start a new file
               if(!file.exists())
               {
                  LoggerManager.resetLoggerFiles(mFilename);
               }
            }
            
            // rotate the log file if necessary
            rotateLogFile(logFileText);
            
            // print to all appropriate streams
            Iterator i = mStreamToVerbosity.keySet().iterator();
            while(i.hasNext())
            {
               // get the next stream and its verbosity
               PrintStream ps = (PrintStream)i.next();
               Integer sv = (Integer)mStreamToVerbosity.get(ps);
               
               if(sv != null)
               {
                  if(sv.intValue() >= verbosity)
                  {
                     try
                     {
                        if(ps == getPrintStream())
                        {
                           ps.println(logFileText);
                        }
                        else
                        {
                           if(ps == System.out || useCustomStreams)
                           {
                              ps.println(logText);
                           }
                        }
                        
                        ps.flush();
                     }
                     catch(Throwable t)
                     {
                        // there was an error with the stream, remove it
                        // from the stream verbosity map
                        i.remove();
                     }
                  }
               }
            }
            
            // if there is any remainder, log it without a logger header
            if(!remainder.equals(""))
            {
               log(remainder, verbosity, false, false);
            }
            
            rval = true;
         }
      }

      return rval;
   }
   
   /**
    * Gets the stack trace from a throwable object.
    * 
    * @param t the throwable object.
    * @return the stack trace as a string.
    */
   public static String getStackTrace(Throwable t)
   {
      StringWriter sw = new StringWriter();
      PrintWriter pw = new PrintWriter(sw);
      t.printStackTrace(pw);
      pw.close();
      
      return sw.toString();
   }   
   
   /**
    * Writes the passed string to the log file, if it is open.
    * Verbosity is set to error level.
    *
    * @param text the text to write to the log file.
    * @return true if the text was written, false if not.
    */
   public boolean error(String text)
   {
      boolean rval = false;
      
      rval = log("ERROR: " + text, ERROR_VERBOSITY);
      
      return rval;
   }

   /**
    * Writes the passed string to the log file, if it is open.
    * Verbosity is set to warning level.
    *
    * @param text the text to write to the log file.
    * @return true if the text was written, false if not.
    */
   public boolean warning(String text)
   {
      boolean rval = false;
      
      rval = log("WARNING: " + text, WARNING_VERBOSITY);
      
      return rval;
   }

   /**
    * Writes the passed string to the log file, if it is open.
    * Verbosity is set to debug level.
    *
    * @param text the text to write to the log file.
    * @return true if the text was written, false if not.
    */
   public boolean debug(String text)
   {
      boolean rval = false;
      
      rval = log("DEBUG: " + text, DEBUG_VERBOSITY);
      
      return rval;
   }
   
   /**
    * Writes the passed string to the log file, if it is open.
    * Verbosity is set to message level.
    *
    * @param text the text to write to the log file.
    * @return true if the text was written, false if not.
    */
   public boolean msg(String text)
   {
      boolean rval;
      
      rval = log("MSG: " + text, MSG_VERBOSITY);
      
      return rval;
   }
}
