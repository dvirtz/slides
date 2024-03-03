import { MainApi } from './preload';

declare global {
  interface Window {
    main: MainApi;
    showCalendar: (year: number, startOfWeek: string) => Promise<void>;
  }
}
