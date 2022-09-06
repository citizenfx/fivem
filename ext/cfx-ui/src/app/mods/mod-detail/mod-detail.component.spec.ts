import { ComponentFixture, TestBed, waitForAsync } from '@angular/core/testing';

import { ModDetailComponent } from './mod-detail.component';

describe('ModDetailComponent', () => {
  let component: ModDetailComponent;
  let fixture: ComponentFixture<ModDetailComponent>;

  beforeEach(waitForAsync(() => {
    TestBed.configureTestingModule({
      declarations: [ ModDetailComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ModDetailComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
