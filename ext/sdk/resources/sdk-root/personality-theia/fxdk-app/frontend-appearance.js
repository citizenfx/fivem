const fxdkLoader = /* svg */`

<svg fill="none" viewBox="0 0 1074 768" width="1074" height="768" xmlns="http://www.w3.org/2000/svg">
	<foreignObject width="100%" height="100%">
		<div xmlns="http://www.w3.org/1999/xhtml">
			<style>
        .booting {
          position: fixed;

          top: 0;
          left: 0;
          right: 0;
          bottom: 0;

          background-color: hsl(226, 23%, 11%);

          transition: all .5s ease;
          opacity: 1;

          pointer-events: none;

          display: flex;
          align-items: center;
          justify-content: center;

          width: 100%;
          height: 100%;

          /* top of the world! */
          z-index: 9999;
        }

        .lds-ripple {
          display: inline-block;
          position: relative;
          width: 80px;
          height: 80px;
        }
        .lds-ripple div {
          position: absolute;
          border: 4px solid #f40552;
          opacity: 1;
          border-radius: 50%;
          animation: lds-ripple 1s cubic-bezier(0, 0.2, 0.8, 1) infinite;
        }
        .lds-ripple div:nth-child(2) {
          animation-delay: -0.5s;
        }
        @keyframes lds-ripple {
          0% {
            top: 36px;
            left: 36px;
            width: 0;
            height: 0;
            opacity: 1;
          }
          100% {
            top: 0px;
            left: 0px;
            width: 72px;
            height: 72px;
            opacity: 0;
          }
        }
			</style>
			<div class="booting">
        <div class="lds-ripple"><div></div><div></div></div>
      </div>
		</div>
	</foreignObject>
</svg>
`;

const fxdkAppearanceCss = /* css */`
.theia-preload {
  width: 100vw !important;
  height: 100vh !important;
  background-image: url("data:image/svg+xml,${encodeURIComponent(fxdkLoader)}") !important;
  background-size: cover !important;
}
`;

module.exports = function attachFxdkAppearance() {
  const cssNode = document.createElement('style');
  cssNode.innerText = fxdkAppearanceCss;
  document.head.appendChild(cssNode);

  document.addEventListener('click', (event) => {
    const target = event.target;

    const isA = target.matches('a');
    const isAChild = target.matches('a *');

    if (!isA && !isAChild) {
      return;
    }

    event.preventDefault();

    const link = isA
      ? target.getAttribute('href')
      : target.closest('a').getAttribute('href');


    if (link) {
      invokeNative('openUrl', link);
    }
  });
}
