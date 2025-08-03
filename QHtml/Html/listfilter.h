/*
	Created By YSP
	2025.7.31
*/
#pragma once
#include <QList>
namespace ysp::qt::html {
	class ListFilter {
	public:
		template <typename T>
		static qint32 Count(const QList<T>& list, const std::function<bool(const T&)>& filter) {
			qint32 count = 0;
			for (const T& item : list) {
				if (filter(item)) ++count;
			}
			return count;
		}

		template <typename T>
		static bool Any(const QList<T>& list, const std::function<bool(const T&)>& filter) {
			for (const T& item : list) {
				if (filter(item)) return true;
			}
			return false;
		}

		template <typename T>
		static bool All(const QList<T>& list, const std::function<bool(const T&)>& filter) {
			for (const T& item : list) {
				if (!filter(item)) return false;
			}
			return true;
		}

		template <typename T>
		static QList<T> Where(const QList<T>& list, const std::function<bool(const T&)>& filter) {
			QList<T> result;
			for (const T& item : list) {
				if (filter(item)) result.append(item);
			}
			return result;
		}

		template <typename T>
		static qint32 Indexof(const QList<T>& list, const std::function<bool(const T&)>& filter) {
			for (qint32 i = 0; i < list.count(); ++i) {
				if (filter(list[i])) return i;
			}
			return -1;
		}

		/// <summary>
		/// 比较函数参数一是当前最大的数，参数二是我们需要比较的数，返回true表示我们需要比较的数较大，返回false表示需要比较的数小
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="list"></param>
		/// <param name="compare"></param>
		/// <returns></returns>
		template <typename T>
		static T Max(const QList<T>& list, const std::function<bool(const T&, const T&)>& compare) {
			if (list.count() <= 0) return T();
			T result = list[0];
			for (qint32 i = 1; i < list.count(); ++i) {
				if (compare(result, list[i]))
					result = list[i];
			}
			return result;
		}

		/// <summary>
		/// 比较函数参数一是当前最小的数，参数二是我们需要比较的数，返回true表示我们需要比较的数较小，返回false表示需要比较的数大
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="list"></param>
		/// <param name="compare"></param>
		/// <returns></returns>
		template <typename T>
		static T Min(const QList<T>& list, const std::function<bool(const T&, const T&)>& compare) {
			if (list.count() <= 0) return T();
			T result = list[0];
			for (qint32 i = 1; i < list.count(); ++i) {
				if (compare(result, list[i])) {
					result = list[i];
				}
			}
			return result;
		}

	};
}