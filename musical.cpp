/*
 * Program: Musical chairs game with n players and m intervals.
 * Author:  K Vamshi Krishna Reddy, P Sai Varshittha
 * Roll# :  CS18BTECH11024, CS18BTECH11035
 */

#include <stdlib.h>  /* for exit, atoi */
#include <iostream>  /* for fprintf */
#include <errno.h>   /* for error code eg. E2BIG */
#include <getopt.h>  /* for getopt */
#include <assert.h>  /* for assert */
#include <chrono>	/* for timers */
#include <utility>
#include <thread>
#include <string.h>
#include <mutex>
#include <condition_variable>
using namespace std;


/*
 *Global Variables
 */
int nplayers;//no. of players
int count=0;//no. of chairs filled
int z=0;
int musicstop=0;//this state is 1 from music_stop to lap_stop
mutex m1, m2, m3, m4;
int player_sleep_times[1000];
bool isSleep[1000];
condition_variable c1, c2, c3;

int musicOn=0;// this state means that, player threads are sleeping/searching for chairs
// this state is 1 from music_start to lap_stop

void usage(int argc, char *argv[]);
unsigned long long musical_chairs(int nplayers);

int main(int argc, char *argv[])
{
    int c;
    /* Loop through each option (and its's arguments) and populate variables */
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] ={
            { "help", no_argument, 0, 'h' },
            { "nplayers", required_argument, 0, '1' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "h1:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            cerr << "option " << long_options[option_index].name;
            if (optarg)
                cerr << " with arg " << optarg << endl;
            break;

        case '1':
            nplayers = atoi(optarg);
            break;

        case 'h':
        case '?':
            usage(argc, argv);

        default:
            cerr << "?? getopt returned character code 0%o ??n" << c << endl;
            usage(argc, argv);
        }
    }

    if (optind != argc) {
        cerr << "Unexpected arguments.\n";
        usage(argc, argv);
    }


    if (nplayers == 0) {
        cerr << "Invalid nplayers argument." << endl;
        return EXIT_FAILURE;
    }

    unsigned long long game_time;
    game_time = musical_chairs(nplayers);

    cout << "Time taken for the game: " << game_time << " us" << endl;

    exit(EXIT_SUCCESS);
}

/*
 * Show usage of the program
 */
void usage(int argc, char *argv[])
{
    cerr << "Usage:\n";
    cerr << argv[0] << "--nplayers <n>" << endl;
    exit(EXIT_FAILURE);
}

void umpire_main(int nplayer)
{
    char temp[20];
    printf("Musical Chairs: %d player game with %d laps.\n", nplayer, nplayer-1);
    fflush(stdout);
    while (1)
    {
        scanf("%s", temp);
        if (!strcmp(temp, "music_start"))
        {
            musicOn=1;
            c2.notify_all();
        }
        else if (!strcmp(temp, "music_stop"))
        {
            musicstop=1;
            unique_lock <mutex> lock1(m3);
            c1.wait(lock1);
        }
        else if (!strcmp(temp, "lap_stop"))
        {
            musicOn=0;
            if (nplayers == 1)
            {
                c2.notify_one();
                /*When only 1 player is left, that player thread is
                signaled and declared as Winner*/
                return;
            }
        }
        else if (!strcmp(temp, "player_sleep"))
        {
            int id, sleep_time;
            scanf("%d", &id);
            scanf("%d", &player_sleep_times[id]);
            isSleep[id]=true;
        }
        else if (!strcmp(temp, "umpire_sleep"))
        {
            int sleep_time;
            scanf("%d", &sleep_time);
            this_thread::sleep_for(chrono::microseconds(sleep_time));
        }
    }
    return;
}

void player_main(int plid)
{
    while (1)
    {
        if (musicOn)
        {
            if (isSleep[plid])
            {
                this_thread::sleep_for(chrono::microseconds(player_sleep_times[plid]));
                isSleep[plid]=false;
            }
            if (musicstop)
            {
                unique_lock<mutex> lock2(m4);
                /*a player thread who gets first chance,
                 will choose an empty chair starting from chair 0 to chair n-1
                 *they choose the chairs in a producer-consumer fashion,*/
                if (nplayers-1>count)
                {
                    count++;
                }
                else
                {
                    /*if count is equal to nplayers-1,
                     it means that all chairs have been filled,
                     so this player thread will be eliminated*/
                    count=0;
                    nplayers--;
                    z++;
                    printf("======= lap# %d =======\n", z);
                    fflush(stdout);
                    printf("%d could not get chair\n", plid);
                    fflush(stdout);
                    printf("**********************\n");
                    fflush(stdout);
                    lock2.unlock();
                    c1.notify_one();
                    musicstop = 0;
                    break;
                }
                c2.wait(lock2);
            }
        }
        if (nplayers == 1)
        {// when only 1 player is left, he is the Winner
            printf("Winner is %d\n", plid);
            fflush(stdout);
            return;
        }
    }
    return;
}//main ended

unsigned long long musical_chairs(int nplayers)
{
    auto t1 = chrono::steady_clock::now();//starting timer

    thread umpire=thread(umpire_main, nplayers);// Spawning umpire thread.

    thread players[nplayers];
    for (int i=0;i<nplayers;i++)// Spawning n player threads.
        players[i] = thread(player_main, i);

    for (auto& th :players)
        th.join();
    umpire.join();

    auto t2 = chrono::steady_clock::now();//ending timer

    auto d1 = chrono::duration_cast<chrono::microseconds>(t2 - t1);
    return d1.count();
}
