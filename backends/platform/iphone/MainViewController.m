//
//  MainViewController.m
//  scummvm
//
//  Created by Yinon Yamin on 3/22/15.
//
//

#import "MainViewController.h"

@interface MainViewController ()

@end

@implementation MainViewController

-(BOOL)prefersStatusBarHidden
{
    return YES;
}

- (BOOL)shouldAutorotate
{

    return NO;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
