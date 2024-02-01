// See the Electron documentation for details on how to use preload scripts:
// https://www.electronjs.org/docs/latest/tutorial/process-model#preload-scripts
import { contextBridge, ipcRenderer } from "electron";

const channels = ['pipe'];

const exposeIpcRenderer = () => {
    contextBridge.exposeInMainWorld("ipcRenderer", {
        send: (channel, data) => {
            if(channels.includes(channel)) {
                ipcRenderer.send(channel, data);
            }
        }, 
        receive: (channel, func) => {
            if(channels.includes(channel)) {
                ipcRenderer.on(channel, (_, ...args) => func(...args));
            }
        }, 
    })
};

exposeIpcRenderer();