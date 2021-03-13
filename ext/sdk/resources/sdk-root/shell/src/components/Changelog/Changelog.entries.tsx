import * as React from 'react';

export interface ChangelogEntry {
  id: string,
  title: React.ReactNode,
  content: React.ReactNode,
}

export const changelogEntries: ChangelogEntry[] = [
  {
    id: '2021-03-13',
    title: 'March 13, 2021',
    content: (
      <>
        <ul>
          <li>
            Issue with different game builds not connecting to server has been fixed
          </li>
        </ul>
      </>
    ),
  },
  {
    id: '2021-03-12',
    title: 'March 12, 2021',
    content: (
      <>
        <ul>
          <li>
            <strong>Ditched embedded filesystem viewer (as seen in project creator) in favor of system's folder select dialog</strong>
            <br/>
            This is due to a big amount of unforeseen edge-cases that sometimes cause viewer to be completely broken.
          </li>
        </ul>
      </>
    ),
  },
  {
    id: 'initial',
    title: 'FxDK Pre-release 🎉',
    content: (
      <>
        <p>
          <strong>23 february 2021</strong>
        </p>
        <ul>
          <li>FxDK now supports recycling deleted items, woo</li>
          <li>Resource rename leading to shell crash got fixed, oops</li>
        </ul>

        <p>
          <strong>17 february 2021</strong>
        </p>
        <ul>
          <li>Project builder just got better, it is now capable of building "complete" server</li>
        </ul>

        <p>
          <strong>12 february 2021</strong>
        </p>
        <ul>
          <li>Right-click to copy selection while in client/server console</li>
        </ul>

        <p>
          <strong>10 february 2021</strong>
        </p>
        <ul>
          <li>You can now drag'n'drop files or folders from desktop to folders/resources in fxdk</li>
          <li>
            Server is running with <kbd>sv_fxdkMode=1</kbd> convar now,
            so you can use different logic for fetching players tokens/identifiers while in fxdk environment.
            <br/>
            Test it like so: <kbd>const isFxdk = GetConvarInt('sv_fxdkMode', 0) == 1</kbd>.
          </li>
        </ul>

        <p>
          <strong>8 february 2021</strong>
        </p>
        <ul>
          <li>Basic project builder</li>
          <li>Theia updated from 1.9.0 to 1.10.0</li>
          <li>Game menu of theia is now "FxDK" with two more options</li>
        </ul>

        <p>
          <strong>3 february 2021</strong>
        </p>
        <ul>
          <li>"Open in Explorer" context menu item available in ProjectExplorer now</li>
        </ul>

        <p>
          <strong>1 february 2021</strong>
        </p>
        <ul>
          <li>Welcome client console as well<br/>(Game -&gt; Toggle Client Console)</li>
        </ul>

        <p>
          <strong>29 january 2021</strong>
        </p>
        <ul>
          <li>Server console moved from server configuration modal to dedicated theia widget<br/>(Game -&gt; Toggle Server Console)</li>
          <li>Server console is now able to accept commands from you right in FxDK, woo</li>
        </ul>

        <p>
          <strong>20 january 2021</strong>
        </p>
        <ul>
          <li>If game crashes it will be restarted automatically, if it was connected to built-in server it will reconnect upon restart as well.</li>
        </ul>

        <p>
          <strong>18 january 2021</strong>
        </p>
        <ul>
          <li>Resource watch commands are now more visible</li>
          <li>You can now see watch commands output (Right click on resource -&gt; Watch commands output)</li>
        </ul>

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
