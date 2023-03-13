#include "CppUnitTest.h"
#include "GuardProcessorMock.h"
#include "WindowInfo.h"
#include <wchar.h>
using namespace testing;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace WindowInfoTest
{
	TEST_CLASS(WindowInfoTest)
	{
	private:
		// Arrange
		std::wstring title = L"Test Window";
		GuardProcessorMock* mockGuardProcessor = new GuardProcessorMock();
	public:
		
		TEST_METHOD(ConstructorTest)
		{
			
			// Act
			WindowInfo windowInfo(title, mockGuardProcessor);

			// Assert
			Assert::AreEqual(title, windowInfo.getTitle());
			Assert::AreEqual(0, windowInfo.getCaret());
			Assert::IsFalse(windowInfo.getStopStatus());
		}

		TEST_METHOD(AddCharTest)
		{
			// Arrange
			WindowInfo windowInfo(title, mockGuardProcessor);

			// Act
			windowInfo.addChar(L"t");
			windowInfo.addChar(L"e");
			windowInfo.addChar(L"s");
			windowInfo.addChar(L"t");

			// Assert
			Assert::AreEqual(std::wstring(L"test"), windowInfo.getStream().str());
			Assert::AreEqual(4, windowInfo.getCaret());

			windowInfo.clearStream();

			// Act
			windowInfo.addChar(L"П");
			windowInfo.addChar(L"р");
			windowInfo.addChar(L"е");
			windowInfo.addChar(L"з");
			windowInfo.addChar(L"и");
			windowInfo.addChar(L"д");
			windowInfo.addChar(L"е");
			windowInfo.addChar(L"н");
			windowInfo.addChar(L"т");

			Assert::AreEqual(std::wstring(L"Президент"), windowInfo.getStream().str());
			Assert::AreEqual(9, windowInfo.getCaret());

			windowInfo.addChar(L" ");
			Assert::AreEqual(std::wstring(L"Президент "), windowInfo.getStream().str());

			windowInfo.clearStream();

			/*
			* Случай, когда у нас буфер больше допустимого и мы нашли стоп слово
			* GuardProcessorMock -> true
			* stopStatus = true
			* alarmState = false
			*/
			EXPECT_CALL(*mockGuardProcessor, process(_, _)).WillOnce(Return(true));
			std::wstring largeString(WORD_BUFFER + 1, L'a');
			for (wchar_t i : largeString) {
				windowInfo.addChar(std::wstring(1, i));
			}		
			Assert::AreEqual(true, windowInfo.getStopStatus());
			Assert::AreEqual(false, windowInfo.getAlarmState());
			
			/*
			* Случай, когда у нас буфер больше допустимого и мы нашли стоп слово и подстоп
			* GuardProcessorMock -> true
			* stopStatus = true
			* alarmState = true
			*/
			EXPECT_CALL(*mockGuardProcessor, process(testing::Eq(true), _)).WillOnce(Return(true));
			windowInfo.addChar(L" ");
			Assert::AreEqual(true, windowInfo.getStopStatus());
			Assert::AreEqual(true, windowInfo.getAlarmState());
			/*
			* При добавлении символа при вышеперечисленом условии 
			* вызовет exception поскольку нериализован логер
			* мы должны скинуть буфер в лог
			*/
			try {
				windowInfo.addChar(L" ");
				Assert::Fail(L"Expected std::runtime_error to be thrown");
			}
			catch (std::runtime_error ex) {
				// Assert
				
				std::stringstream str;
				str << ex.what();
				std::string s = str.str();
				std::wstring result(s.begin(), s.end());
				Assert::AreEqual(std::wstring(L"Error: Need adding log to logger file"), result);
				Assert::AreEqual(0, windowInfo.getCaret());
				Assert::AreEqual(std::wstring(), windowInfo.getStream().str());
				Assert::IsFalse(windowInfo.getStopStatus());

				/*
				* Случай, когда у нас буфер больше допустимого и мы не нашли стоп слово
				* GuardProcessorMock -> false
				* stopStatus = false
				* alarmState = false
				*/
				EXPECT_CALL(*mockGuardProcessor, process(_, _)).WillOnce(Return(false));
				for (wchar_t i : largeString) {
					windowInfo.addChar(std::wstring(1, i));
				}
				Assert::AreEqual(false, windowInfo.getStopStatus());
				Assert::AreEqual(false, windowInfo.getAlarmState());
				Assert::AreEqual(std::wstring(), windowInfo.getStream().str());
				try {
					//windowInfo.addChar(L" ");
					//Assert::Fail(L"Expected std::runtime_error to be thrown");
				}
				catch (std::runtime_error ex) {
					// Assert
					std::stringstream str;
				}
			}
		}

		TEST_METHOD(moveLeft) {
			WindowInfo windowInfo(L"hello", mockGuardProcessor);
			windowInfo.addChar(L"t");
			windowInfo.addChar(L"e");
			windowInfo.addChar(L"s");
			windowInfo.addChar(L"t");

			windowInfo.moveLeft();
			Assert::AreEqual(3, windowInfo.getCaret());
			windowInfo.moveLeft();
			Assert::AreEqual(2, windowInfo.getCaret());
			windowInfo.moveLeft();
			Assert::AreEqual(1, windowInfo.getCaret());
			windowInfo.moveLeft();
			Assert::AreEqual(1, windowInfo.getCaret());
		}

		TEST_METHOD(moveRight) {
			WindowInfo windowInfo(L"hello", mockGuardProcessor);
			windowInfo.addChar(L"t");
			windowInfo.addChar(L"e");
			windowInfo.addChar(L"s");
			windowInfo.addChar(L"t");

			windowInfo.moveRight();
			Assert::AreEqual(4, windowInfo.getCaret());
			windowInfo.moveLeft();
			Assert::AreEqual(3, windowInfo.getCaret());
			windowInfo.moveLeft();
			Assert::AreEqual(2, windowInfo.getCaret());
			windowInfo.moveLeft();
			Assert::AreEqual(1, windowInfo.getCaret());
			windowInfo.moveLeft();
			Assert::AreEqual(1, windowInfo.getCaret());
			windowInfo.moveRight();
			Assert::AreEqual(2, windowInfo.getCaret());
			windowInfo.moveRight();
			Assert::AreEqual(3, windowInfo.getCaret());
			windowInfo.moveRight();
			Assert::AreEqual(4, windowInfo.getCaret());
			windowInfo.moveRight();
			Assert::AreEqual(4, windowInfo.getCaret());
		}

		TEST_METHOD(removeChar) {
			// Arrange
			WindowInfo windowInfo(L"hello", mockGuardProcessor);
			windowInfo.addChar(L"t");
			windowInfo.addChar(L"e");
			windowInfo.addChar(L"s");
			windowInfo.addChar(L"t");
			/*
			* каретка в конце - удаляем вправо должно ничего не произойти
			*/
			windowInfo.removeChar(1);
			Assert::AreEqual(4, windowInfo.getCaret());
			Assert::AreEqual(std::wstring(L"test"), windowInfo.getStream().str());

			windowInfo.moveLeft();
			windowInfo.moveLeft();
			windowInfo.moveLeft();
			windowInfo.moveLeft();

			/*
			* каретка вначале - удаление влево должно ничего не произойти
			*/
			windowInfo.removeChar(-1);
			Assert::AreEqual(1, windowInfo.getCaret());
			Assert::AreEqual(std::wstring(L"test"), windowInfo.getStream().str());

			/*
			* каретка вначале - удаление вправо - удалится первый элемент
			*/
			windowInfo.removeChar(1);
			Assert::AreEqual(1, windowInfo.getCaret());
			Assert::AreEqual(std::wstring(L"est"), windowInfo.getStream().str());

			windowInfo.moveRight();
			windowInfo.moveRight();
			windowInfo.moveRight();
			/*
			* каретка вконце - удаление влево - удалится последний элемент
			*/
			windowInfo.removeChar(-1);
			Assert::AreEqual(2, windowInfo.getCaret());
			Assert::AreEqual(std::wstring(L"es"), windowInfo.getStream().str());
			/*
			* каретка вконце - удаление влево - удалятся все элементы
			*/
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			Assert::AreEqual(0, windowInfo.getCaret());
			Assert::AreEqual(std::wstring(L""), windowInfo.getStream().str());

			/*
			* каретка вначале - удаление вправо - удалятся все элементы
			*/
			windowInfo.addChar(L"t");
			windowInfo.addChar(L"e");
			windowInfo.addChar(L"s");
			windowInfo.addChar(L"t");
			windowInfo.moveLeft();
			windowInfo.moveLeft();
			windowInfo.moveLeft();
			windowInfo.moveLeft();
			windowInfo.moveLeft();
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(1);
			Assert::AreEqual(0, windowInfo.getCaret());
			Assert::AreEqual(std::wstring(L""), windowInfo.getStream().str());

			/*
			* каретка в середине - удаление влево - удалятся все элементы
			*/
			windowInfo.addChar(L"t");
			windowInfo.addChar(L"e");
			windowInfo.addChar(L"s");
			windowInfo.addChar(L"t");
			windowInfo.moveLeft();
			windowInfo.moveLeft();
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			Assert::AreEqual(0, windowInfo.getCaret());
			Assert::AreEqual(std::wstring(L""), windowInfo.getStream().str());


			/*
			* каретка в середине - удаление вправо - удалятся все элементы
			*/
			windowInfo.addChar(L"t");
			windowInfo.addChar(L"e");
			windowInfo.addChar(L"s");
			windowInfo.addChar(L"t");
			windowInfo.moveLeft();
			windowInfo.moveLeft();
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			Assert::AreEqual(0, windowInfo.getCaret());
			Assert::AreEqual(std::wstring(L""), windowInfo.getStream().str());
		}

		TEST_METHOD(removeCharTab) {
			WindowInfo windowInfo(L"hello", mockGuardProcessor);
			windowInfo.addChar(L"П");
			windowInfo.addChar(L"р");
			windowInfo.addChar(L"и");
			windowInfo.addChar(L"в");
			windowInfo.addChar(L"е");
			windowInfo.addChar(L"т");
			windowInfo.addChar(L" ");
			windowInfo.addChar(L"М");
			windowInfo.addChar(L"И");
			windowInfo.addChar(L"Р");
			windowInfo.addChar(L"\t");
			windowInfo.addChar(L"!");
			windowInfo.addChar(L"\n");
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.removeChar(-1);
			windowInfo.addChar(L"М");
			windowInfo.addChar(L"И");
			windowInfo.addChar(L"Р");
			windowInfo.addChar(L"\t");
			windowInfo.addChar(L"!");
			windowInfo.addChar(L"\n");
			windowInfo.addChar(L"\n");
			windowInfo.addChar(L"\n");
			windowInfo.addChar(L"\n");
		}
	};
}
