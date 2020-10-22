declare function attachShader(gl: any, program: any, type: any, src: any): any;
declare function compileAndLinkShaders(gl: any, program: any, vs: any, fs: any): void;
declare function createTexture(gl: any): any;
declare function createBuffers(gl: any): {
    vertexBuff: any;
    texBuff: any;
};
declare function createProgram(gl: any): {
    program: any;
    vloc: any;
    tloc: any;
};
declare function createGameView(): {
    canvas: HTMLCanvasElement;
    gl: WebGLRenderingContext | null;
    animationFrame: undefined;
    resize: (width: any, height: any) => void;
    destroy: () => void;
};
declare function mapMouseButton(button: any): any;
declare function mapKey(which: any, location: any): any;
declare function isLMB(e: any): boolean;
declare const vertexShaderSrc: "\n  attribute vec2 a_position;\n  attribute vec2 a_texcoord;\n  uniform mat3 u_matrix;\n  varying vec2 textureCoordinate;\n  void main() {\n    gl_Position = vec4(a_position, 0.0, 1.0);\n    textureCoordinate = a_texcoord;\n  }\n";
declare const fragmentShaderSrc: "\nvarying highp vec2 textureCoordinate;\nuniform sampler2D external_texture;\nvoid main()\n{\n  gl_FragColor = texture2D(external_texture, textureCoordinate);\n}\n";
declare class GameView extends HTMLElement {
    set mode(arg: number);
    get mode(): number;
    _mode: number;
    get isObservingMode(): boolean;
    get isControlingMode(): boolean;
    mouseMoveMultiplier: number;
    _keysState: any[];
    _buttonsState: any[];
    _acceptInput: boolean;
    _acceptMouseButtons: boolean;
    _pointerLocked: boolean;
    _fullscreenActive: boolean;
    _observingModeActiveKeys: Set<any>;
    _observingModeActiveMouseButtons: Set<any>;
    gameView: {
        canvas: HTMLCanvasElement;
        gl: WebGLRenderingContext | null;
        animationFrame: undefined;
        resize: (width: any, height: any) => void;
        destroy: () => void;
    };
    _canvas: HTMLCanvasElement;
    _hint: HTMLElement;
    /**
     * @lifecycle
     */
    connectedCallback(): void;
    /**
     * @lifecycle
     */
    disconnectedCallback(): void;
    /**
     * @api
     */
    enterFullscreenControlingMode(): void;
    /**
     * @api
     */
    lockPointer(): boolean;
    /**
     * @api
     */
    enterFullscreen(): boolean;
    _addEventListeners(): void;
    _removeEventListeners(): void;
    _setupResizeObserver(): void;
    _resizeObserver: any;
    _resetStates(): void;
    _createHandlers(): void;
    _handleWindowBlur: (() => void) | undefined;
    _handlePointerLockChange: (() => void) | undefined;
    _handleFullscreenChange: (() => void) | undefined;
    _handleDocumentMouseMove: ((e: any) => void) | undefined;
    _handleMousedown: ((e: any) => void) | undefined;
    _handleMouseup: ((e: any) => void) | undefined;
    _handleMousewheel: ((e: any) => void) | undefined;
    _handleMousemove: ((e: any) => void) | undefined;
    _handleKeydown: ((e: any) => void) | undefined;
    _handleKeyup: ((e: any) => void) | undefined;
}
declare namespace GameView {
    const ModeControling: number;
    const ModeObserving: number;
}
//# sourceMappingURL=game-view.webcomponent.d.ts.map