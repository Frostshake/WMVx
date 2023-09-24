#pragma once

#include <QString>
#include <QFile>
#include <qtcsv/reader.h>

namespace core {

	class BuildInfoFileReader : protected QtCSV::Reader::AbstractProcessor {
	public:
		BuildInfoFileReader(QString fileName) {
			headerFound = false;
			valid = true;

			QFile source(fileName);
			if (source.open(QIODevice::ReadOnly)) {
				QtCSV::Reader::readToProcessor(source, *this, "|");
				source.close();
			}
		}
		virtual ~BuildInfoFileReader() {}

		bool isValid() const {
			return valid;
		}

		QString getVersion() const {
			auto index = fieldIndex("Version!STRING:0");

			if(index >= 0) {
				if (rows.size() > 0) {
					return rows[0][index];
				}
			}

			return "";
		}

	protected:

		bool processRowElements(const QList<QString>& elements) override {
			if (!headerFound) {
				headerFound = true;

				valid = elements.size() <= MAX_ELEMENTS;

				if (valid) {
					header = elements;
				}
			} else if (rows.size() < MAX_ROWS) {

				valid = elements.size() == header.size();

				if (valid) {
					rows.push_back(elements);
				}
			}
			else {
				valid = false;
			}

			return valid;
		}

		int32_t fieldIndex(QString headerName) const {
			auto it = std::find(header.cbegin(), header.cend(), headerName);

			if (it != header.cend()) {
				return it - header.cbegin();
			}

			return -1;
		}

		QStringList header;
		std::vector<QStringList> rows;

	private:
		bool headerFound;
		bool valid;


		// unsure of expected column / row count, do basic checks to avoid bad data.
		static constexpr int32_t MAX_ELEMENTS = 25;
		static constexpr int32_t MAX_ROWS = 10;
	};

}