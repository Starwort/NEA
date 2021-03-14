# Final testing and evaluation

## Usability Testing

Testing has been carried out with one of the stakeholders using Windows; a recording has been made of this testing (see below).

<video controls>
<source src="http://starbright.dyndns.org/starwort/nea/stakeholder_test.mkv" type="video/mkv" />

Sorry, your browser doesn't support embedded videos. View it here:
[Recording of testing](http://starbright.dyndns.org/starwort/nea/stakeholder_test.mkv)

</video>
<br />

Stakeholders have given feedback, listed below:

- id-05012e5c-1462-4510-9fa3-e7e92b1062a9
  - Feedback:
    - An indication of the game number in the status bar would be helpful; it’s not possible to see which game of n you are on at any time - are there 4 more games to go or 40? Given that the unsuccessful solves don’t count, it’s impossible to say.
    - Solve time could be logged and displayed in the status bar. This would be interesting and give you an idea of whether changes made to the parameters actually improve or impair the speed of attempts.
    - Information about the number of failed attempts, average moves in successful attempts, and average time for solution could be tracked and displayed in a dialogue at the end of the sequence of games. A report showing this in an easily readable format would be useful.
    - An indication of what the optimal numbers for the various options are would give the user more information about how to make changes and why.
    - Steam achievements are good to get, especially “unobtainable” ones, but I think I would want this as a last resort for that final achievement, not as a goal. I would like the ability to apply it to other card games though.
    - I do enjoy card games but solving it myself is the fun part. Is the aim to prove that an “unsolvable” one really is? If the games have numbers can one force the solver to tackle a particular game?
  - Bugs demonstrated during testing:
    - Corner flinging to stop solution execution does not update button text accordingly.
    - Completion of desired number of games does not update button text.

  This feedback is useful because it highlights that the stakeholder has ideas for additional features that had not previously been thought of, and that the stakeholder may have preferred an alternate usage where the program provides a hint towards the current puzzle, instead of simply solving it

## Effective Evaluation

## Maintenance

The solver application has been tested on Windows and Linux and should also work on MacOS but none of my testers use MacOS. It doesn’t require installation, and requires only three Python libraries installed with pip, the Python package manager. This can be done easily by anyone with simple instructions.

The application does not require regular updates; updates would only be required when new functionality is added. It would not require modifications to continue with current features as it does not require an internet connection to function.

The application does not require backups as there is no data storage requirement - it does not need to be installed, it does not save any persistent data as its state is maintained only during runtime, and therefore there would be nothing to back up.

There is no archive requirement for this application as there is no data to be stored, and therefore the footprint of the solver is very small indeed. It has no persistent state.

Future technology developments that may enhance and improve the system include: improved GUI systems; better algorithms; improved input emulation; and any significant improvement in processing speed would improve the performance of the solver.

## Incomplete Systems and Further Development

If the application was further developed in the future, additional features that would improve the experience include improved algorithms, more detailed information in the user interface, and a playable standalone solitaire game - not using the Steam game - which uses the assets within the repository and uses the solver code to provide hints. Adding functionality to learn more games would require significantly more development. If there was more time available for this particular iteration, the standalone game could have been added, but teaching it more games would require much more time and effort.

The solver works reasonably well as it is, but some aspects could be improved:

- The command-line solver does not have a time limit and therefore it can take an extremely long time to find a solution, depending on the board and its settings. The GUI uses a subprocess timeout to kill the solver, having assumed it did not finish, which means that the solver taking far too long is a non-issue when using the GUI. But because the user can choose to only use the command-line executable, the lack of timeout causes delay and friction  in the process. You cannot force the user to choose the GUI (although it makes the process easier) - it is not possible to prevent the command-line from being used as the user can pass the same arguments that the GUI uses; it also has a help message, containing the information detailed [here](https://starwort.github.io/NEA/all.html#solver-arguments) which tells the user how to pass the options.
- The input emulation does not lock the user’s mouse; therefore the user can disrupt the solution while it is being executed. This results in the solver moving cards into incorrect places, which will prevent the solution from being executed correctly (and the game will not be solved).
