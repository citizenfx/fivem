import { Component, OnInit } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';

@Component({
  selector: 'app-create-editor',
  templateUrl: './create-editor.component.html',
  styleUrls: ['./create-editor.component.scss']
})
export class CreateEditorComponent implements OnInit {
  welcomeMessage: any;

  constructor(private domSanitizer: DomSanitizer) { }

  ngOnInit(): void {
    this.fetchEditor();
  }

  fetchEditor() {
    window.fetch('https://runtime.fivem.net/create_editor.html')
          .then(async res => {
              if (res.ok) {
                  this.welcomeMessage = this.domSanitizer.bypassSecurityTrustHtml(await res.text());
              }
          });
  }

  clickContent(event: MouseEvent) {
    const srcElement = event.srcElement as HTMLElement;

    if (srcElement.localName === 'button') {
      (<any>window).invokeNative('executeCommand', 'replayEditor');
      event.preventDefault();
    }
  }

}
