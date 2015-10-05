/* Justinas Linkevicius
 * IFF-3/2
 * L1a
 *

1. Kokia tvarka startuoja procesai?
**** tokia, kokia užrašyti

2. Kokia tvarka vykdomi procesai?
**** atsitiktine

3. Kiek iteracijų iš eilės padaro vienas procesas?
**** vieną pilnai

4. Kokia tvarka atspausdinami to paties masyvo duomenys?
****  atsitiktine.

*/

package linkeviciusj_l1a;

import java.io.*;
import static java.lang.Math.ceil;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Justinas
 */
public class Linkevicius_L1a
{
    // duomenu failas
    final String fileName = "src/linkeviciusj_l1a/LinkeviciusJ.txt";
    
    // giju kiekis
    int processCount = 10;
    
    // lauku pavadinimai
    String fieldNames[]  = new String[3];
    
    // duomenu elementu kiekis
    int dataElementCount = 0;
    
    // kiekis elementu, kuriuos tures apdoroti kiekviena gija
    int threadDataSize   = 0;
    
    // duomenu masyvai gijoms
    LinkeviciusJ_DataFile[][] dataArrays;
    
    private void readFile()
    {
        try
        {
            BufferedReader input = new BufferedReader(new FileReader(this.fileName));

            String line;
            String var1;
            int var2;
            double var3;

            line = input.readLine();
            String [] parts =  line.split("\\s+");
            
            // gauna lauku pavadinimus
            fieldNames[0] = parts[0];
            fieldNames[1] = parts[1];
            fieldNames[2] = parts[2];
            
            // gauna duomenu kieki
            this.dataElementCount = Integer.parseInt(parts[3]);
            
            // apskaiciuoja kiek elementu reikes apdoroti kiekvienai gijai
            this.threadDataSize = (int) ceil( (double) this.dataElementCount / this.processCount );
            
            System.out.println("this.dataElementCount = " + this.dataElementCount);
            System.out.println("this.processCount = " + this.processCount);
            System.out.println("this.threadDataSize = " + this.threadDataSize);
            
            dataArrays = new LinkeviciusJ_DataFile[this.processCount][];
            
            int currentLine = 0;
            for (int i = 0; i < this.processCount; i++)
            {
                // sukuriamas masyvas gijai
                dataArrays[i] = new LinkeviciusJ_DataFile[this.threadDataSize];
                
                int j = 0;
                while( (line = input.readLine()) != null)
                {
                    String [] tokens =  line.split("\\s+");
                    var1 = tokens[0];
                    var2 = Integer.parseInt(tokens[1]);
                    var3 = Double.parseDouble(tokens[2]);
                    
                    // jei masyvui nebera duomenu, uzpildome tusciais elementais
                    if(currentLine < this.dataElementCount)
                        dataArrays[i][j] = new LinkeviciusJ_DataFile(var1, var2, var3);
                    else
                        dataArrays[i][j] = new LinkeviciusJ_DataFile();
                    
                    currentLine++;
                    j++;
                    
                    if(j == this.threadDataSize)
                        break;
                }
            }
           
            input.close();
        } catch (IOException e)
        {
            System.out.println(e.getMessage());
        }
    }
    
    // atsausdina pradinius duomenis
    private void writeData()
    {
       int n = 1;
       System.out.printf("\n%-20s %-5s %-5s\r\n", this.fieldNames[0], this.fieldNames[1], this.fieldNames[2]);
       for (int i = 0; i < this.processCount; i++)
       {
           System.out.println("\n" + "**** Array " + i + " ****");
           for(int j = 0; j < this.threadDataSize; j++)
           {
               if( dataArrays[i][j] != null )
               {
                    System.out.print(j + ") ");
                    System.out.printf("%-20s %-5d %-5.2f", dataArrays[i][j].getStringField(), dataArrays[i][j].getIntField(),  dataArrays[i][j].getDoubleField());
                    System.out.println();
                    n++;
               }
           }
       }
       System.out.println();
    }
    
    private void startMultithreading()
    {
        System.out.println("Start of multithreading!");
        Linkevicius_L1a_Thread[] threads = new Linkevicius_L1a_Thread[this.processCount];
        
        for(int i = 0; i < this.processCount; i++)
        {
            threads[i] = new Linkevicius_L1a_Thread(this.dataArrays[i], this.threadDataSize, i);
            threads[i].start();
        }
        
        for(int i = 0; i < this.processCount; i++)
        {
            try {
                threads[i].join();
            } catch (InterruptedException ex)
            {
                Logger.getLogger(Linkevicius_L1a.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        
        System.out.println("End of program!");
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args)
    {
        Linkevicius_L1a L1a = new Linkevicius_L1a();
        
        L1a.readFile();
        L1a.writeData();
        L1a.startMultithreading();
    }
}

class Linkevicius_L1a_Thread extends Thread
{
    // duomenys gijoms
    private LinkeviciusJ_DataFile[] dataArray;
    private int arraySize;
    private int threadIndex;

    Linkevicius_L1a_Thread(LinkeviciusJ_DataFile[] a, int b, int c)
    {
        dataArray   = a;
        arraySize   = b;
        threadIndex = c;
    }

    // lygiagreciai vykdomas programos metodas
    public void run()
    {
        for (int i = 0; i < arraySize; i++)
        {
            if(dataArray[i] != null)
                System.out.println(String.format("%-10s%-4s%-20s%-5s%-8s", "procesas_"+threadIndex, " " + i, dataArray[i].getStringField(), dataArray[i].getIntField(), dataArray[i].getDoubleField()));
        }
    }
}

/*
 * Klase skirta saugoti pradiniams duomenims
 */
class LinkeviciusJ_DataFile
{
    private String stringField;
    private int    intField;
    private double doubleField;
    
    LinkeviciusJ_DataFile(String a, int b, double c)
    {
        stringField = a;
        intField    = b;
        doubleField = c;
    }

    LinkeviciusJ_DataFile()
    {
        stringField = "";
        intField    = 0;
        doubleField = 0.0;
    }

    public String getStringField()
    {
        return stringField;
    }

    public int getIntField()
    {
        return intField;
    }

    public double getDoubleField()
    {
        return doubleField;
    }
    
    public void setStringField(String string_field)
    {
        this.stringField = string_field;
    }

    public void setIntField(int int_field)
    {
        this.intField = intField;
    }

    public void setDoubleField(double doubleField)
    {
        this.doubleField = doubleField;
    }
}
