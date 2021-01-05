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
          <strong>14 january 2021</strong>
        </p>
        <ul>
          <li>Resource templates in resource creator</li>
          <li>
            C# template enabled autobuild-on-change for .cs files in projects
            <br />
            (you need .NET installed to create resource with C# template)
          </li>
          <li>
            Specify <kbd>fxdk_watch_command</kbd> in your <kbd>fxmanifest.lua</kbd>,
            <br />
            FxDK will run it if resource is enabled AND restart-on-change is on
          </li>
        </ul>

        <p>
          <strong>30 december 2020</strong>
        </p>
        <ul>
          <li>Full CSharp language support (w/o proper resource creation, yet)</li>
          <li>Move theia plugins out of theia distribution package, as omnisharp extension downloads <em>a lot</em> of files afterwards</li>
        </ul>

        <p>
          <strong>22 december 2020</strong>
        </p>
        <ul>
          <li>FxDK-flavored ayu theme by default</li>
          <li>Check if 30120 port is available for built-in fxserver upon start</li>
        </ul>

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
