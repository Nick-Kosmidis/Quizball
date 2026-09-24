# Quizball

**Quizball** is an interactive football trivia game developed in **Unreal Engine**. It is designed for endless fun whether you are playing solo, challenging a friend in a 1v1 match, or teaming up with groups for an ultimate football quiz night!

The game is available for both **PC (Windows)** and **Android** mobile devices.

---

## Game Modes
Enjoy the game in multiple ways:
* **Solo:** Test your personal football knowledge.
* **1v1:** Go head-to-head with a friend.
* **Teams:** Split into groups and compete against each other in a trivia night.

---

## Categories
The game features a rich variety of fixed and dynamic categories:

### Fixed Categories
* **History:** Historical events, tournaments, and moments from the past.
* **Geography:** Football geography, countries, and stadiums.
* **Top5:** Find Top 5 players on a category.
* **Whos Missing:** Find out which player is missing from a lineup.
* **Manager ID:** Recognize managers based on their career history.
* **Player ID:** Guess the football player.
* **Gossip:** Fun facts and football news off the pitch.

### Dynamic Categories
* **Higher-Lower:** Which player has more goals, assists, or market value?
* **Club Combo:** Combinations of players who played in both teams.
* **Guess the Score:** Predict the correct scores and scorers of matches.

---

## Scoring System & Power-ups
* **Difficulty & Points:** Each category is divided into sub-questions based on difficulty, awarding from **1 to 3 points**.
* **Power-ups:**
  * **x2:** Doubles the points of a question, but **must be used before seeing the question**.
  * **50-50:** Removes half of the incorrect answers.

---

## Downloads

You can download the latest version of Quizball below:

* **PC (Windows):** [Download Quizball for PC (.zip)](https://drive.google.com/file/d/16L0sv01mEnwpUirFUOLYaMGtdCi7H-Ng/view?usp=sharing)
* **Mobile (Android):** [Download Quizball for Android (.apk)](https://drive.google.com/file/d/1pZelxhICU7-uWsjzIzO3iiN2SgqDIBY8/view?usp=sharing)

---

## Export Instructions (For Developers)

If you are a developer and want to package the project yourself from Unreal Engine for PC or Android, follow these steps:

1. Open the project in **Unreal Engine**.
2. Go to the top menu: **File > Package Project**.
3. Select your target platform:
   * **For PC:** Select `Windows` (or `Windows (64-bit)`). Choose your output directory and wait for the packaging process to complete.
   * **For Mobile (Android):** Select `Android` (usually `Android ASTC` or `Multi`). Make sure you have properly configured the Android SDK/NDK in Project Settings -> Android SDK if you are building an `.apk`.
4. Locate the exported files:
   * For PC: Zip the packaged folder for distribution.
   * For Android: Retrieve the generated `.apk` file.
