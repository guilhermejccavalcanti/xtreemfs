/*  Copyright (c) 2008 Konrad-Zuse-Zentrum fuer Informationstechnik Berlin.

 This file is part of XtreemFS. XtreemFS is part of XtreemOS, a Linux-based
 Grid Operating System, see <http://www.xtreemos.eu> for more details.
 The XtreemOS project has been developed with the financial support of the
 European Commission's IST program under contract #FP6-033576.

 XtreemFS is free software: you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation, either version 2 of the License, or (at your option)
 any later version.

 XtreemFS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with XtreemFS. If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * AUTHORS: Jan Stender (ZIB)
 */

package org.xtreemfs.mrc.operations;

import java.util.Iterator;

import org.xtreemfs.common.logging.Logging;
import org.xtreemfs.interfaces.Constants;
import org.xtreemfs.interfaces.DirectoryEntry;
import org.xtreemfs.interfaces.DirectoryEntrySet;
import org.xtreemfs.interfaces.stat_;
import org.xtreemfs.interfaces.MRCInterface.readdirRequest;
import org.xtreemfs.interfaces.MRCInterface.readdirResponse;
import org.xtreemfs.mrc.ErrNo;
import org.xtreemfs.mrc.ErrorRecord;
import org.xtreemfs.mrc.MRCRequest;
import org.xtreemfs.mrc.MRCRequestDispatcher;
import org.xtreemfs.mrc.UserException;
import org.xtreemfs.mrc.ErrorRecord.ErrorClass;
import org.xtreemfs.mrc.ac.FileAccessManager;
import org.xtreemfs.mrc.database.AtomicDBUpdate;
import org.xtreemfs.mrc.database.StorageManager;
import org.xtreemfs.mrc.metadata.FileMetadata;
import org.xtreemfs.mrc.utils.MRCHelper;
import org.xtreemfs.mrc.utils.Path;
import org.xtreemfs.mrc.utils.PathResolver;
import org.xtreemfs.mrc.volumes.VolumeManager;
import org.xtreemfs.mrc.volumes.metadata.VolumeInfo;

/**
 * 
 * @author stender
 */
public class ReadDirAndStatOperation extends MRCOperation {
    
    public static final int OP_ID = 12;
    
    public ReadDirAndStatOperation(MRCRequestDispatcher master) {
        super(master);
    }
    
    @Override
    public void startRequest(MRCRequest rq) {
        
        try {
            
            final readdirRequest rqArgs = (readdirRequest) rq.getRequestArgs();
            
            final VolumeManager vMan = master.getVolumeManager();
            final FileAccessManager faMan = master.getFileAccessManager();

            validateContext(rq);
            
            Path p = new Path(rqArgs.getPath());
            
            VolumeInfo volume = vMan.getVolumeByName(p.getComp(0));
            StorageManager sMan = vMan.getStorageManager(volume.getId());
            PathResolver res = new PathResolver(sMan, p);
            
            // check whether the path prefix is searchable
            faMan.checkSearchPermission(sMan, res, rq.getDetails().userId, rq.getDetails().superUser, rq
                    .getDetails().groupIds);
            
            // check whether file exists
            res.checkIfFileDoesNotExist();
            
            FileMetadata file = res.getFile();
            
            // if the file refers to a symbolic link, resolve the link
            String target = sMan.getSoftlinkTarget(file.getId());
            if (target != null) {
                rqArgs.setPath(target);
                p = new Path(target);
                
                // if the local MRC is not responsible, send a redirect
                if (!vMan.hasVolume(p.getComp(0))) {
                    finishRequest(rq, new ErrorRecord(ErrorClass.USER_EXCEPTION, ErrNo.ENOENT,
                        "link target " + target + " does not exist"));
                    return;
                }
                
                volume = vMan.getVolumeByName(p.getComp(0));
                sMan = vMan.getStorageManager(volume.getId());
                res = new PathResolver(sMan, p);
                file = res.getFile();
            }
            
            // check whether the directory grants read access
            faMan.checkPermission(FileAccessManager.O_RDONLY, sMan, file, res.getParentDirId(), rq
                    .getDetails().userId, rq.getDetails().superUser, rq.getDetails().groupIds);
            
            AtomicDBUpdate update = sMan.createAtomicDBUpdate(master, rq);
            
            // if required, update POSIX timestamps
            if (!master.getConfig().isNoAtime())
                MRCHelper.updateFileTimes(res.getParentDirId(), file, true, false, false, sMan, update);
            
            DirectoryEntrySet dirContent = new DirectoryEntrySet();
            
            Iterator<FileMetadata> it = sMan.getChildren(res.getFile().getId());
            while (it.hasNext()) {
                
                FileMetadata child = it.next();
                
                String linkTarget = sMan.getSoftlinkTarget(child.getId());
                int mode = faMan.getPosixAccessMode(sMan, child, rq.getDetails().userId,
                    rq.getDetails().groupIds);
                mode |= linkTarget != null ? Constants.SYSTEM_V_FCNTL_S_IFLNK : child.isDirectory() ? Constants.SYSTEM_V_FCNTL_S_IFDIR : Constants.SYSTEM_V_FCNTL_S_IFREG;
                long size = linkTarget != null ? linkTarget.length() : child.isDirectory() ? 0 : child
                        .getSize();
                int type = linkTarget != null ? 3 : child.isDirectory() ? 2 : 1;
                stat_ stat = new stat_(mode, child.getLinkCount(), 1, 1, 0, size, child.getAtime(), child
                        .getMtime(), child.getCtime(), child.getOwnerId(), child.getOwningGroupId(), volume
                        .getId()
                    + ":" + child.getId(), linkTarget, child.getEpoch(), (int) child.getW32Attrs());
                // TODO: check whether Win32 attrs are 32 or 64 bits long
                
                dirContent.add(new DirectoryEntry(child.getFileName(), stat));
            }
            
            // set the response
            rq.setResponse(new readdirResponse(dirContent));
            
            update.execute();
            
        } catch (UserException exc) {
            Logging.logMessage(Logging.LEVEL_TRACE, this, exc);
            finishRequest(rq, new ErrorRecord(ErrorClass.USER_EXCEPTION, exc.getErrno(), exc.getMessage(),
                exc));
        } catch (Exception exc) {
            finishRequest(rq, new ErrorRecord(ErrorClass.INTERNAL_SERVER_ERROR, "an error has occurred", exc));
        }
    }
    
}
