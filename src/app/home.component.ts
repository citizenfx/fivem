import { Component, OnInit } from '@angular/core';

@Component({
    moduleId: module.id,
    selector: 'app-home',
    template: `
    <div class="tab">
    home is... where the heart is?!
    </div>
`
})

export class HomeComponent implements OnInit {
    constructor() { }

    ngOnInit() { }
}