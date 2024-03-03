// See the Electron documentation for details on how to use preload scripts:
// https://www.electronjs.org/docs/latest/tutorial/process-model#preload-scripts
import { contextBridge, ipcRenderer } from 'electron';

interface MainApi {
  generateCalendar: (year: number, startOfWeek: number) => Promise<string[]>;
}

const api: MainApi = {
  generateCalendar: (year: number, startOfWeek: number) => ipcRenderer.invoke('generateCalendar', year, startOfWeek)
}

contextBridge.exposeInMainWorld('main', api)
