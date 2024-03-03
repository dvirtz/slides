import bindings from 'pkg-prebuilds';
import bindingOptions from './binding-options';
import * as path from 'path';

interface CalendarGenerator {
  // Return an array of strings representing the calendar
  // Each string represents a month or a week in the calendar
  generateCalendar: (year: number, startOfWeek: number) => string[];
}

export const { generateCalendar } = bindings<CalendarGenerator>(path.join(__dirname, '..', '..'),
  bindingOptions);
