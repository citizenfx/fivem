import { Component, OnInit } from '@angular/core';
import { GameService } from "app/game.service";

@Component({
  selector: 'app-exit',
  templateUrl: './exit.component.html',
  styleUrls: ['./exit.component.scss']
})
export class ExitComponent implements OnInit {

  constructor(private gameServer: GameService) { }

  ngOnInit() {
  }

  onExit(){
    this.gameServer.exitGame();
    
  }


}
