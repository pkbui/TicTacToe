// TODO(Khang):
// - Turn game into class, with methods
// - Remove unnecessary variables
// - Make a "Process turn" function to run in game loop
// - PostGame state and timer to restart game
// - Fix the player first thing, that's not needed anymore

#include<SFML/Window.hpp>
#include<SFML/Graphics.hpp>
#include<iostream>
#include <random>
#include <chrono>

int cellSize = 100;
int gridDim = cellSize * 3;
sf::Font font;

enum class Winner{PLAYER, MACHINE, NONE};
enum class Coin{HEAD, TAIL};
enum class CORNER { ULEFT, URIGHT, DLEFT, DRIGHT };

enum class Turn
{
	Player,
	Machine
};

enum class GameState
{
	TurnSelection,
	TurnResult,
	Gameplay
};

struct Game
{
	GameState currentState;

	Turn currentTurn;
	Coin coinResult;
	Winner winner;
	
	int playBoard[3][3] = {};
	
	std::chrono::time_point<std::chrono::steady_clock> startWaitTime;
	bool waiting;

	std::vector<CORNER> availCorner = { CORNER::ULEFT, CORNER::URIGHT, CORNER::DLEFT, CORNER::DRIGHT };
	std::vector<sf::Vector2f> OCoords;
	std::vector<sf::Vector2f> XCoords;

};


void drawBoard(sf::RenderWindow& window) {
	for (auto i = 0; i < gridDim; i += cellSize) {
		sf::Vertex line[] = { sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(i))), sf::Vector2f(static_cast<float>(gridDim), static_cast<float>(i)) };
		sf::Vertex line2[] = { sf::Vertex(sf::Vector2f(static_cast<float>(i), 0.0f)), sf::Vector2f(static_cast<float>(i), static_cast<float>(gridDim)) };
		window.draw(line, 2, sf::PrimitiveType::Lines);
		window.draw(line2, 2, sf::PrimitiveType::Lines);
	}
}

void drawCircle(sf::RenderWindow& window, int x, int y) {
	sf::CircleShape circle(cellSize / 3);
	circle.setPosition(sf::Vector2f(static_cast<float>((x * cellSize) + 15), static_cast<float>((y*cellSize) + 15) ));
	circle.setFillColor(sf::Color::Transparent);
	circle.setOutlineThickness(10);
	circle.setOutlineColor(sf::Color::White);
	window.draw(circle);
}

void drawCross(sf::RenderWindow& window, int x, int y) {
	sf::RectangleShape line1(sf::Vector2f(100, 5));
	sf::RectangleShape line2(sf::Vector2f(100, 5));
	line1.setPosition(sf::Vector2f(static_cast<float>((x * cellSize) + 15), static_cast<float>((y*cellSize) + 15)));
	line2.setPosition(sf::Vector2f(static_cast<float>((x * cellSize) + 85), static_cast<float>((y*cellSize) + 20)));
	line1.rotate(45);
	line2.rotate(135);
	window.draw(line1);
	window.draw(line2);
}

sf::Vector2f getCellCoord(int mouseX, int mouseY) {
	auto relativeX = 0;
	auto relativeY = 0;
	for (int i = 0; i <= 2; i++) {
		if ( (i*cellSize <= mouseX) && (mouseX <= (i + 1) * cellSize) ) {
			relativeX = i;
		}
		if ((i*cellSize <= mouseY) && (mouseY <= (i + 1) * cellSize)) {
			relativeY = i;
		}
	}
	return sf::Vector2f(static_cast<float>(relativeX), static_cast<float>(relativeY));
}

// TODO(Khang) make this take const Game, shouldn't modify game for this
sf::Vector2f getMachineMove(Game& game) {
	sf::Vector2f nextMove;
	std::random_device randDevice;
	int cell = 0;
	auto x = 0;
	auto y = 0;
		do {
			if (game.availCorner.size() != 0u) {
				std::shuffle(std::begin(game.availCorner), std::end(game.availCorner), randDevice);
				switch (game.availCorner.at(0))
				{
				case CORNER::ULEFT:
					cell = game.playBoard[0][0];
					x = 0;
					y = 0;
					break;
				case CORNER::URIGHT:
					cell = game.playBoard[0][2];
					x = 0;
					y = 2;
					break;
				case CORNER::DLEFT:
					cell = game.playBoard[2][0];
					x = 2;
					y = 0;
					break;
				case CORNER::DRIGHT:
					cell = game.playBoard[2][2];
					x = 2;
					y = 2;
					break;
				default:
					break;
				}
				game.availCorner.erase(game.availCorner.begin());
			}
			else {
				break;
			}
		} while (cell != 0);
	if(game.availCorner.size() == 0u) {
		std::uniform_int_distribution<int> gridDist(0, 2);
		do {
			x = gridDist(randDevice);
			y = gridDist(randDevice);
		} while (game.playBoard[x][y] != 0);
	}
	nextMove = sf::Vector2f(static_cast<float>(x), static_cast<float>(y));
	return nextMove;
}

Winner getWinner(const Game& game) {
	Winner winner = Winner::NONE;
	for (int i = 0; i < 3; i++) {
		//check cells on same row
		if ((game.playBoard[i][0] == game.playBoard[i][1])
			&& (game.playBoard[i][1] == game.playBoard[i][2])
			) {
			if (game.playBoard[i][0] == 1) {
				winner = Winner::PLAYER;
			}
			else if (game.playBoard[i][0] == 2) {
				winner = Winner::MACHINE;
			}
			else {
				winner = Winner::NONE;
			}
		}
		//check cells on same col
		if ((game.playBoard[0][i] == game.playBoard[1][i])
			&& (game.playBoard[1][i] == game.playBoard[2][i])
			) {
			if (game.playBoard[0][i] == 1) {
				winner = Winner::PLAYER;
			}
			else if (game.playBoard[0][i] == 2) {
				winner = Winner::MACHINE;
			}
			else {
				winner = Winner::NONE;
			}
		}
	}
	if( (game.playBoard[0][0] == game.playBoard[1][1])
		&& (game.playBoard[1][1] == game.playBoard[2][2])) {
		if (game.playBoard[1][1] == 1) {
			winner = Winner::PLAYER;
		}
		else if (game.playBoard[1][1] == 2) {
			winner = Winner::MACHINE;
		}
		else {
			winner = Winner::NONE;
		}
	}
	if ((game.playBoard[0][2] == game.playBoard[1][1])
		&& (game.playBoard[1][1] == game.playBoard[2][0])) {
		if (game.playBoard[1][1] == 1) {
			winner = Winner::PLAYER;
		}
		else if (game.playBoard[1][1] == 2) {
			winner = Winner::MACHINE;
		}
		else {
			winner = Winner::NONE;
		}
	}
	return winner;
}

bool boardCellAvailable(const Game& game) {
	bool cellAvail = false;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			if (game.playBoard[x][y] == 0) {
				cellAvail = true;
				return cellAvail;
			}
		}
	}
	return cellAvail;
}

void setPlayOrder(Coin playerChoice) {
	std::random_device randDevice;
	std::uniform_int_distribution<int> coinFlip(0, 1);
	Coin result = Coin::HEAD;
	if (coinFlip(randDevice) == 0)
	{
		result = Coin::TAIL;
	}
	playerFirst = (result == playerChoice);

}

bool playerGoFirst() {
	return playerFirst;
}

void drawState(sf::RenderWindow& window, const Game& game) {
	window.clear();
	sf::Text text;
	text.setFont(font);
	text.setColor(sf::Color::Red);
	text.setCharacterSize(24);

	switch (game.currentState)
	{
	case GameState::TurnSelection:
	{
		text.setString("Choose head or tail \nby pressing H or T.\nGuess correctly to go first");
	} break;

	case GameState::TurnResult:
	{
		if (playerGoFirst()) {
			text.setString("Guess is correct.\nPlayer goes first");
		}
		else {
			text.setString("Guess is wrong. \nMachine goes first");
		}

		
	} break;

	case GameState::Gameplay:
	{
		drawBoard(window);
		for (auto c : game.OCoords) {
			drawCircle(window, c.x, c.y);
		}
		for (auto c : game.XCoords) {
			drawCross(window, c.x, c.y);
		}
		switch (getWinner(game))
		{
		case Winner::PLAYER:
			text.setString("Player won");
			break;
		case Winner::MACHINE:
			text.setString("Machine won");
			break;
		case Winner::NONE:
			if (!boardCellAvailable(game)) {
				text.setString("It's a tie");
			}
			break;
		default:
			break;
		}
	} break;

	default:
		break;
	}

	if (text.getString() != "") {
		window.draw(text);
	}
	window.display();
}

int main()
{
	
	if (!font.loadFromFile("arial.ttf"))
	{
		int x = 0;
	}
	sf::RenderWindow window(sf::VideoMode(gridDim, gridDim), "TicTacToe");

	Game game;
	game.currentState = GameState::TurnSelection;
	game.waiting = false;

	while (window.isOpen()) {
		sf::Event event;
		sf::Vector2f mouseCoord;

		if (game.waiting)
		{
			auto endTime = std::chrono::steady_clock::now();
			auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - game.startWaitTime);
			if (timeElapsed.count() > 500)
			{
				game.currentState = GameState::Gameplay;
				game.waiting = false;
			}
		}

		while (window.pollEvent(event)) {

			switch (event.type){
			case sf::Event::MouseButtonPressed:
			{
				if (boardCellAvailable(game) &&
					(getWinner(game) == Winner::NONE)) {
					if (event.mouseButton.button == sf::Mouse::Left) {
						auto currentO = getCellCoord(event.mouseButton.x, event.mouseButton.y);
						if (game.playBoard[static_cast<int>(currentO.x)][static_cast<int>(currentO.y)] == 0) {
							game.OCoords.push_back(currentO);
							game.currentTurn = Turn::Machine;
							game.playBoard[static_cast<int>(currentO.x)][static_cast<int>(currentO.y)] = 1;
						}
					}
				}
				break;
			}
			case sf::Event::KeyPressed:
			{
				if (game.currentState == GameState::TurnSelection) {
					if (event.key.code == sf::Keyboard::H) {
						setPlayOrder(Coin::HEAD);
					}
					else if (event.key.code == sf::Keyboard::T) {
						setPlayOrder(Coin::TAIL);
					}
					game.startWaitTime = std::chrono::steady_clock::now();
					game.waiting = true;
					game.currentState = GameState::TurnResult;

					if (!playerGoFirst())
					{
						game.currentTurn = Turn::Machine;
					}
				}
				break;
			}
			case sf::Event::Closed:
			{
				window.close();
				break;
			}
			default:
				break;
			}

			if (game.currentState == GameState::Gameplay && game.currentTurn == Turn::Machine) {
				if (boardCellAvailable(game) &&
					(getWinner(game) == Winner::NONE)) {
					sf::Vector2f newXCoord = getMachineMove(game);
					game.XCoords.push_back(newXCoord);
					game.playBoard[static_cast<int>(newXCoord.x)][static_cast<int>(newXCoord.y)] = 2;
					game.currentTurn = Turn::Player;
				}
			}
		}	
		drawState(window, game);
	}
	return 0;
}