package org.xtreemfs.interfaces.DIRInterface;

import java.io.StringWriter;
import org.xtreemfs.*;
import org.xtreemfs.common.buffer.ReusableBuffer;
import org.xtreemfs.interfaces.*;
import org.xtreemfs.interfaces.utils.*;
import yidl.runtime.Marshaller;
import yidl.runtime.PrettyPrinter;
import yidl.runtime.Struct;
import yidl.runtime.Unmarshaller;




public class xtreemfs_replication_to_masterRequest extends org.xtreemfs.interfaces.utils.Request
{
    public static final int TAG = 2009121017;
    
    public xtreemfs_replication_to_masterRequest() {  }

    // java.lang.Object
    public String toString() 
    { 
        StringWriter string_writer = new StringWriter();
        string_writer.append(this.getClass().getCanonicalName());
        string_writer.append(" ");
        PrettyPrinter pretty_printer = new PrettyPrinter( string_writer );
        pretty_printer.writeStruct( "", this );
        return string_writer.toString();
    }

    // Request
    public Response createDefaultResponse() { return new xtreemfs_replication_to_masterResponse(); }


    // java.io.Serializable
    public static final long serialVersionUID = 2009121017;    

    // yidl.runtime.Object
    public int getTag() { return 2009121017; }
    public String getTypeName() { return "org::xtreemfs::interfaces::DIRInterface::xtreemfs_replication_to_masterRequest"; }
    
    public int getXDRSize()
    {
        int my_size = 0;

        return my_size;
    }    
    
    public void marshal( Marshaller marshaller )
    {

    }
    
    public void unmarshal( Unmarshaller unmarshaller ) 
    {
    
    }
        
        

}
