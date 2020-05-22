import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'app-story-home',
  templateUrl: './story-home.component.html',
  styleUrls: ['./story-home.component.scss']
})
export class StoryHomeComponent implements OnInit {

  constructor() { }

  ngOnInit(): void {
  }

  goStory() {
    (<any>window).invokeNative('executeCommand', 'storymode');
  }
}
