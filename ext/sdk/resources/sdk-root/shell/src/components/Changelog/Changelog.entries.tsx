import * as React from 'react';

export interface ChangelogEntry {
  id: string,
  title: React.ReactNode,
  content: React.ReactNode,
}

export const changelogEntries: ChangelogEntry[] = [
  {
    id: 'initial',
    title: 'FxDK Pre-release 🎉',
    content: (
      <>
        <p>
          <strong>21 december 2020</strong>
        </p>
        <ul>
          <li>Show toolbar immediately after project open/create process begins</li>
          <li>Copy-paste functionality for folders and files</li>
        </ul>

        <p>
          <strong>18 december 2020</strong>
        </p>
        <ul>
          <li>Compact FxDK toolbar design, allowing more space for project explorer</li>
          <li>Granular-ish resources state propagation to shell-client</li>
          <li>Some tasks are now reporting their state including server installer bits</li>
        </ul>

        <p>
          <strong>16 december 2020</strong>
        </p>
        <ul>
          <li>Welcome changelog</li>
          <li>Improved preparing screen</li>
          <li>Lots of internal work as well</li>
        </ul>
      </>
    ),
  },
];
