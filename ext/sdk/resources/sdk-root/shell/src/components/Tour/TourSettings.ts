export const introOptions = {
  doneLabel: 'Done',
  showBullets: false,
  hidePrev: true,
};

export const steps = [
  {
    element: '.root',
    title: 'Welcome to Fxdk',
    intro: 'Fxdk is a code editor for FiveM. Create servers and resources with ease. Export and build all the files you will need to run a server when you are done.',
    tooltipClass: 'customTooltip',
    highlightClass: 'highlightClass'
  },
  {
    element: 'button[title="New Resource"]',
    title: 'Create New Resources',
    intro: 'Create new resources for your server. Try one of our starting templates for Lua, JS, or C#.',
    tooltipClass: 'customTooltip',
    highlightClass: 'highlightClass'
  },
  {
    element: 'div[title="Toolbar"]',
    title: 'File Explorer',
    intro: 'When you create a new resource they will appear here in the file directory. Click here to open files to edit or create new files. Right click on resource folders to start, stop, or restart the game asset.',
    tooltipClass: 'customTooltip',
    highlightClass: 'highlightClass'
  },
  {
    element: 'div[title="Start server"]',
    title: 'Start Your Server',
    intro: 'This will open a Game View for you to test your resources as you develop them. It opens as a tab in the code editor.',
    tooltipClass: 'customTooltip',
    highlightClass: 'highlightClass'
  },
  {
    element: 'iframe[title="Theia personality"]',
    title: 'The Code Editor',
    intro: 'This section is for editing your files and testing in the Game View. Make adjustments and then restart your resources to test.',
    tooltipClass: 'customTooltip',
    highlightClass: 'highlightClass'
  },
  {
    element: 'button[title="Build project"]',
    title: 'Build Project',
    intro: 'When you are done editing your resources, build your server. Add Tebex store keys and Steam API keys. This will export everything you need to launch your customized server on FiveM!',
    tooltipClass: 'customTooltip',
    highlightClass: 'highlightClass'
  },
];
